import sys
from gtts import gTTS

def text_to_speech(text, filename='output.mp3'):
    tts = gTTS(text=text, lang='ko')
    tts.save(filename)

if __name__ == "__main__":
    text = sys.argv[1]
    text_to_speech(text)
