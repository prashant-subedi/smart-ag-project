"""
Implements AZ IOT Central commands that can be sent to the devices
"""
import requests
import os

auth_api_token=os.getenv("AUTH_API_TOKEN")

def _command_url(command_name):
    return f"https://smart-agriculture.azureiotcentral.com/api/devices/2ot2wjgghpi/commands/startirrigation?api-version=1.0" 

def _send_command(command_name):
    resp = requests.post(
        _command_url(command_name),
        json={"body": "asdsdsa"},
        headers={"Authorization": auth_api_token}    
    )
    print(resp.json())
    resp.raise_for_status()

def start_irrigation():
    _send_command("startirrigation")



