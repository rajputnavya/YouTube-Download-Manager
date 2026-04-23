import yt_dlp
from flask import Flask, request, jsonify, Response
import json
import threading

app = Flask(__name__)
progress_data = {}
lock = threading.Lock()

def download_video_worker(video_link, progress_hook):
    try:
        # Extract available formats
        ydl_opts_format = {
            'listformats': True
        }
        with yt_dlp.YoutubeDL(ydl_opts_format) as ydl:
            info_dict = ydl.extract_info(video_link, download=False)
            formats = info_dict.get('formats', [])

        # Choose the first available format
        if formats:
            first_format = formats[0]['format_id']  # Get the first format
            print(f"Selected format: {first_format}")
        else:
            raise Exception("No formats available")

        # Download using the selected format
        ydl_opts_download = {
            'format': first_format,  # First available format
            'progress_hooks': [progress_hook],
            'outtmpl': 'downloaded_video.%(ext)s'
        }

        with yt_dlp.YoutubeDL(ydl_opts_download) as ydl:
            ydl.download([video_link])

    except Exception as e:
        with lock:
            progress_data['error'] = str(e)

@app.route('/get_video_info', methods=['POST'])
def get_video_info():
    data = request.get_json()
    video_link = data['video_link']

    ydl_opts = {
        'format': 'bestvideo+bestaudio/best'
    }

    try:
        with yt_dlp.YoutubeDL(ydl_opts) as ydl:
            info_dict = ydl.extract_info(video_link, download=False)
            video_title = info_dict.get('title', 'Unknown Title')

            formats = info_dict.get('formats', [])
            first_format = formats[0] if formats else None

            return jsonify({
                'title': video_title,
                'format': first_format['format_id'] if first_format else 'Unknown Format'
            })
    except Exception as e:
        return jsonify({'error': str(e)}), 500
    # return jsonify({"title": "Test Video"})

@app.route('/download_video', methods=['POST'])
def download_video():
    data = request.get_json()
    video_link = data['video_link']

    def progress_hook(d):
        with lock:
            if d['status'] == 'downloading':
                progress_data['downloaded'] = d['downloaded_bytes'] / (1024 * 1024)
                progress_data['total'] = d['total_bytes'] / (1024 * 1024)
                progress_data['speed'] = d['speed'] / 1024 if d['speed'] else 'Unknown'
                progress_data['eta'] = d['eta'] if d['eta'] else 'Unknown'
                progress_data['percent'] = d['_percent_str']
            elif d['status'] == 'finished':
                progress_data['finished'] = True

    thread = threading.Thread(target=download_video_worker, args=(video_link, progress_hook))
    thread.start()

    def generate():
        while thread.is_alive():
            with lock:
                if 'error' in progress_data:
                    yield f"data:{json.dumps({'error': progress_data['error']})}\n\n"
                    break
                yield f"data:{json.dumps(progress_data)}\n\n"
            threading.Event().wait(1)

        with lock:
            if 'finished' in progress_data:
                yield f"data:{json.dumps({'finished': True})}\n\n"

    return Response(generate(), content_type='text/event-stream')

if __name__ == '__main__':
    app.run(port=5000)
