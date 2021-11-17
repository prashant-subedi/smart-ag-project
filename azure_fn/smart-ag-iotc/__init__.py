import logging
import os
import json

import azure.functions as func

# Sentry for error loggning
import sentry_sdk
from sentry_sdk.integrations.serverless import serverless_function

sentry_dsn = os.getenv("dsn")

sentry_sdk.init(dsn=sentry_dsn)

@serverless_function
def main(req: func.HttpRequest) -> func.HttpResponse:
    logging.info('Python HTTP trigger function processed a request.')
    payload = json.loads(req.get_body())

    logging.info(f"{payload}")

    name = req.params.get_body(req)

    return func.HttpResponse(
            f"{req.get_body()}",
            status_code=200
    )
