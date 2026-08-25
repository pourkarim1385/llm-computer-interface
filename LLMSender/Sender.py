import json
from openai import OpenAI
import os

sys_detail: str = None
sys_promt: str = None
user_message: str = None
sys_rols: str = None

with open("detal.txt", "r") as file:
    sys_detail = file.read()

if os.path.exists("detail.txt"):
    os.remove("detail.txt")


response = client.chat.completions.create(
    model="deepseek-v4-flash-vision-exp",
    messages=[
        {
            "role": "user",
            "content": [
                {"type": "text", "text": user_message},
                {
                    # here will be the imga eof the guy
                },
            ],
        },
            {
            "role": "system",
            "content": [
                {"type": "text", "text": sys_detail.join(sys_promt).join(sys_rols)},
                {
                    "type": "image_url",
                    "image_url": {"url": f"data:image/jpeg;base64,{b64}"},
                },
            ],
        }
    ],
)