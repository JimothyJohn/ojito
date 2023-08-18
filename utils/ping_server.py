import requests
import json
import os
from dotenv import load_dotenv

load_dotenv()


def ping_server():
    url = "http://10.0.0.31/predictions"
    headers = {
        "Content-Type": "application/json",
        # TODO Add from env before committing...
        "Authorization": f"Token {os.getenv('REPLICATE_TOKEN')}",
    }
    with open("docs/taylor.txt", "r") as f:
        image = f"data:image/jpeg;base64,{f.read()}"
        f.close()

    data = {"input": {"image": image}}
    response = json.loads(requests.post(url=url, headers=headers, data=json.dumps(data)).text)
    print(response["output"])


ping_server()