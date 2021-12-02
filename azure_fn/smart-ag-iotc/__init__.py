import logging
import os
import json

import azure.functions as func
# Sentry for error loggning
import sentry_sdk
from sentry_sdk.integrations.serverless import serverless_function


from . import weather_api

sentry_dsn = os.getenv("sentry_dsn")

sentry_sdk.init(dsn=sentry_dsn)

@serverless_function
def main(req: func.HttpRequest) -> func.HttpResponse:
    logging.info('Python HTTP trigger function processed a request.')
    
    sensor_data = json.loads(req.get_body())
    weather_data = weather_api.fetch()

    logging.info(f"sensor_data: {sensor_data}")
    logging.info(f"weather_data: {weather_api.fetch()}")

    name = req.params.get_body(req)

    return func.HttpResponse(
            f"{req.get_body()}",
            status_code=200
    )
