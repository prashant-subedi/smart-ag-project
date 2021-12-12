import json
import os

import psycopg2

os_config = os.getenv("database_config")
def save_sensor_value(sensor_data):
    with psycopg2.connect(**json.loads(os_config)) as conn:
        with conn.cursor() as c:
            c.execute(
                """
                    insert into sensor_data(
                        node_id, packet_id, error_code, temp, light, moisture, humidity
                    ) values (%(node_id)s, %(packet_id)s, %(error_code)s, %(temp)s, %(light)s, %(moisture)s, %(humidity)s)
                """, 
                sensor_data
            )





