import requests
import base64
import json
import os
from dotenv import load_dotenv

load_dotenv()


def ping_server():
    url = "http://localhost/predictions"
    headers = {
        "Content-Type": "application/json",
        # TODO Add from env before committing...
        "Authorization": f"Token {os.getenv('REPLICATE_TOKEN')}",
    }
    with open("utils/taylor.txt", "r") as f:
        image = f"data:image/jpeg;base64,{f.read()}"
        f.close()

    data = {"input": {"image": image}}
    response = requests.post(url=url, headers=headers, data=json.dumps(data))
    print(response.text)


ping_server()
