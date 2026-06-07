import sys
import io
import os
import time
import argparse
import numpy as np

# Add hailo-apps to path
HAILO_APPS_PATH = os.path.join(os.path.dirname(os.path.dirname(os.path.abspath(__file__))), "hailo-apps")
sys.path.insert(0, HAILO_APPS_PATH)

def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("--audio", required=True, help="Path to audio file")
    args = parser.parse_args()

    # suppress all verbose output
    old_stdout = sys.stdout
    sys.stdout = io.StringIO()

    try:
        from hailo_apps.python.standalone_apps.speech_recognition.speech_recognition import (
            VARIANT_MODELS, _setup_imports
        )
        from hailo_apps.python.standalone_apps.speech_recognition.whisper_pipeline import WhisperPipeline
        from hailo_apps.python.standalone_apps.speech_recognition.audio_utils import load_audio, preprocess_audio, improve_audio, SAMPLE_RATE
        from hailo_apps.python.standalone_apps.speech_recognition.postprocessing import clean_transcription

        (resolve_arch, _, resolve_hef_paths, WHISPER_H8_APP, RESOURCES_ROOT, NPY_DIR) = _setup_imports()

        arch = "hailo8"
        variant = "base"
        encoder_name, decoder_name = VARIANT_MODELS[variant][arch]

        resolved = resolve_hef_paths(hef_paths=[encoder_name, decoder_name], app_name=WHISPER_H8_APP, arch=arch)
        encoder_path = str(resolved[0].path)
        decoder_path = str(resolved[1].path)

        from pathlib import Path
        npy_dir = Path(RESOURCES_ROOT) / NPY_DIR

        pipeline = WhisperPipeline(encoder_path, decoder_path, variant=variant, npy_dir=str(npy_dir), add_embed=True)
        chunk_length = pipeline.get_chunk_length()

        audio = load_audio(args.audio)
        audio, start_time = improve_audio(audio)

        if start_time is None:
            sys.stdout = old_stdout
            print("")
            return

        offset = max(start_time - 0.2, 0)
        mels = preprocess_audio(audio, chunk_length=chunk_length, chunk_offset=offset)

        results = []
        for mel in mels:
            pipeline.send_data(mel)
            time.sleep(0.1)
            text = pipeline.get_transcription()
            results.append(text)

        full_text = clean_transcription(" ".join(results))
        pipeline.stop()

    except Exception as e:
        sys.stdout = old_stdout
        print(f"Error: {e}", file=sys.stderr)
        sys.exit(1)

    # restore stdout and print ONLY the transcription
    sys.stdout = old_stdout
    print(full_text.strip())

if __name__ == "__main__":
    main()