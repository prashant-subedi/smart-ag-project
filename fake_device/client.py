import os

from dotenv import load_dotenv
from iotc import IoTCClient, IOTCConnectType

load_dotenv()

device_id = os.getenv('DEVICE_ID')
scope_id = os.getenv('ID_SCOPE')
sas_key = os.getenv('SAS_KEY')

iotc = IoTCClient(
    device_id, 
    scope_id,
    IOTCConnectType.IOTC_CONNECT_SYMM_KEY, 
    sas_key
)

iotc.set_model_id('2ot2wjgghpi')

iotc.connect()

iotc.send_telemetry({
    "temperature": 2.2
})