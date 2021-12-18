"""
    Script that runs on the Rasperry PI to make it act as a gateway. 
    It maintains a connection with IOT Central using the iotc library and a connection with SAMD21 with serial
"""
import os
import json
import asyncio
import atexit

from dotenv import load_dotenv
from iotc.aio import IoTCClient
from iotc import IOTCConnectType, IOTCEvents
from iotc.models import Command
import serial
from serial.tools import list_ports

load_dotenv()
<<<<<<< Updated upstream
dev = next(list_ports.grep("/dev/cu.usbmodem1101")) # Get the first device with USB
ser = serial.Serial(dev.device)  
=======

dev = next(list_ports.grep("/dev/cu.usbmodem1101")) # for mac, change for PI
ser = serial.Serial(dev.device)

>>>>>>> Stashed changes

atexit.register(ser.close)


async def read_serial():
    loop = asyncio.get_running_loop()
    return await loop.run_in_executor(None, ser.readline)

async def write_serial(data):
    loop = asyncio.get_running_loop()
    await loop.run_in_executor(None, ser.write, data.encode('utf-8'))

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


async def on_commands(command: Command):
    if command.name == "startirrigation":
        print("Sending command")
        await write_serial(json.dumps({'node_id': 1})+"\n")
        await command.reply()

async def program_loop():
    """
    Sets hander to be executed when command is called and continuiously polls serial for new messags
    Messages in serial are transmitted to cloud and commands from cloud are sent to serial 
    for connected microcontroller to read
    """
    client = await create_client()
    client.on(IOTCEvents.IOTC_COMMAND, on_commands)
    while not client.terminated():
        from_serial = await read_serial()
        try:
            data = json.loads(from_serial)
            await write_serial(
                json.dumps(
                {'node_id': data['node_id'], 'packet_id': data['packet_id']}
            ))
<<<<<<< Updated upstream

            await client.send_telemetry(data)
=======
            if last_of_node.get(data['packet_id']) == data['packet_id']:
                # Dedupe incase of retransmission
                print("DUPLICATE")
                continue
            await client.send_telemetry(data)
            last_of_node[data['node_id']] = data['packet_id']
>>>>>>> Stashed changes
        except BaseException as e:
            print(from_serial)
            print(e)

async def main():
    await program_loop()

if __name__=="__main__":
    asyncio.run(main())
