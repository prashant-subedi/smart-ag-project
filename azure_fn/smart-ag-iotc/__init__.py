import logging
import os
import json

import azure.functions as func
# Sentry for error loggning
import sentry_sdk
from sentry_sdk.integrations.serverless import serverless_function


from . import weather_api
from . import device_command
from . import database

sentry_dsn = os.getenv("sentry_dsn")

sentry_sdk.init(dsn=sentry_dsn)

@serverless_function
def main(req: func.HttpRequest) -> func.HttpResponse:
    logging.info('Python HTTP trigger function processed a request.')
    
    sensor_data = req.get_json()
#     weather_data = weather_api.fetch()
    logging.info(f'{sensor_data}')
    
    database.save_sensor_value(sensor_data)

    if sensor_data["telemetry"]["moisture"] < 0.6:
            device_command.start_irrigation()
            
    logging.info(f"sensor_data: {sensor_data}")
    logging.info(f"weather_data: {weather_api.fetch()}")

    return func.HttpResponse(
            json.dumps(sensor_data),
            status_code=200
    )
