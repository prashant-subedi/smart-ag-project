"""
Entry file for Azure Function
"""
import logging
import os
import json

import azure.functions as func
import sentry_sdk
from sentry_sdk.integrations.serverless import serverless_function


from . import weather_api
from . import device_command

sentry_dsn = os.getenv("sentry_dsn")

sentry_sdk.init(dsn=sentry_dsn)

@serverless_function
def main(req: func.HttpRequest) -> func.HttpResponse:
    """
    Entry function that is called whenever the Azure function is invoked.
    Note: Fetching from weather is removed
    """
    logging.info('Python HTTP trigger function processed a request.')
    
    sensor_data = req.get_json()
#     weather_data = weather_api.fetch()
    logging.info(f'{sensor_data}')

    if sensor_data["telemetry"]["moisture"] < 51:
            device_command.start_irrigation()

    logging.info(f"sensor_data: {sensor_data}")
    logging.info(f"weather_data: {weather_api.fetch()}")

    return func.HttpResponse(
            json.dumps(sensor_data),
            status_code=200
    )
