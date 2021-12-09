import requests

auth_api_token="SharedAccessSignature sr=1f016e66-2f17-406d-a41f-7cd139896bbb&sig=XEeBhxMfKQk4rRZGWD2MU314HSH9kfD0MilP3LScJKA%3D&skn=az-fn&se=1670003104703"

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



