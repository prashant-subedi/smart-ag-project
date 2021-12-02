import os
import sys
import asyncio
import time 

from dotenv import load_dotenv
from iotc.aio import IoTCClient
from iotc import IOTCConnectType, IOTCEvents

load_dotenv()

async def create_client():
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
    await iotc.connect()
    return iotc


async def on_commands(command):
    print("Received command {} with value {}".format(command.name, command.value))
    await command.reply()

async def listen_commands():
    client = await create_client()
    client.on(IOTCEvents.IOTC_COMMAND, on_commands)
    while not client.terminated():
        await asyncio.sleep(1)

async def main():
    await listen_commands()

if __name__=="__main__":
    asyncio.run(main())
