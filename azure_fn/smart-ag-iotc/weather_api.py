import os

import requests

openweather_api_key = os.getenv("openweather_api_key")

URL = f"https://api.openweathermap.org/data/2.5/onecall?lat=33.44&lon=-94.04&exclude=minutely&appid={openweather_api_key}"

def fetch():
    resp = requests.get(URL)
    resp.raise_for_status() # Any non 200 response will raise an exception
    return resp.json()

if __name__=="__main__":
    fetch()