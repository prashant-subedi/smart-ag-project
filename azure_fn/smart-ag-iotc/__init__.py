"""
Entry file for Azure Function
"""
import logging
import os
import json

import azure.functions as func
import sentry_sdk
from sentry_sdk.integrations.serverless import serverless_function


from . import device_command
from . import database
from . import email_client
from . import model

sentry_dsn = os.getenv("sentry_dsn")

sentry_sdk.init(dsn=sentry_dsn)

@serverless_function
def main(req: func.HttpRequest) -> func.HttpResponse:
    """
    Entry function that is called whenever the Azure function is invoked.
    Note: Fetching from weather is removed
    """
    logging.info('Python HTTP trigger function processed a request.')
    
    sensor_data = req.get_json()["telemetry"]
    logging.info(f'{sensor_data}')
    
    database.save_sensor_value(sensor_data)
    predicted_sm = model.predict_SM(sensor_data) 
    if predicted_sm < 74:
        logging.info(f"START IRRIGATION VALUE: {predicted_sm}")
        device_command.start_irrigation()
        try:
            email_client.notify_irrigation_started()
        except BaseException:
            logging.exception("Failed to send email")
    else:
        logging.info(f"NO IRRIGATION")
        
        
    logging.info(f"sensor_data: {sensor_data}")

    return func.HttpResponse(
            json.dumps(sensor_data),
            status_code=200
    )
