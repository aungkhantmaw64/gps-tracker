import json
import threading
import paho.mqtt.client as mqtt
from collections import deque
from dash import Dash, dcc, html
from dash.dependencies import Output, Input
import plotly.graph_objs as go

# MQTT Configuration
BROKER = "test.mosquitto.org"
PORT = 1883
TOPIC = "/egress/ESP_01"

# Shared data buffers
maxlen = 100
latitudes = deque(maxlen=maxlen)
longitudes = deque(maxlen=maxlen)
batteries = deque(maxlen=maxlen)
timestamps = deque(maxlen=maxlen)
latest_data = {"id": "", "date": "", "time": ""}


# MQTT Callbacks
def on_connect(client, userdata, flags, rc):
    print("Connected with result code", rc)
    client.subscribe(TOPIC)


def on_message(client, userdata, msg):
    global latest_data
    try:
        payload = json.loads(msg.payload.decode())
        lat = int(payload["payload"][0:4], 16)
        lng = int(payload["payload"][4:8], 16)
        bat = int(payload["payload"][8:10], 16)

        latitude = (float(lat) / 65535.0) * 180.0 - 90.0
        longitude = (float(lng) / 65535.0) * 360.0 - 180.0
        battery = (float(bat) / 255.0) * 100.0

        latest_data = {
            "id": payload["id"],
            "lat": str(round(latitude, 3)),
            "lng": str(round(longitude, 3)),
            "bat": str(round(battery, 3)),
            "date": payload["date"],
            "time": payload["time"],
        }

        latitudes.append(latitude)
        longitudes.append(longitude)
        batteries.append(battery)
        timestamps.append(payload["time"])

        print(latest_data)

    except Exception as e:
        print("Error processing message:", e)


# Start MQTT in background thread
def start_mqtt():
    client = mqtt.Client()
    client.on_connect = on_connect
    client.on_message = on_message
    client.connect(BROKER, PORT, 60)
    client.loop_forever()


mqtt_thread = threading.Thread(target=start_mqtt, daemon=True)
mqtt_thread.start()

# Dash App
app = Dash(__name__)

app.layout = html.Div(
    [
        html.H2("Live MQTT Telemetry Dashboard"),
        html.Div(id="text-info", style={"marginBottom": "20px"}),
        dcc.Graph(id="live-graph"),
        dcc.Interval(
            id="interval-component",
            interval=1000,  # update every 1 second
            n_intervals=0,
        ),
    ]
)


@app.callback(
    [Output("live-graph", "figure"), Output("text-info", "children")],
    [Input("interval-component", "n_intervals")],
)
def update_graph(n):
    if not timestamps:
        return go.Figure(), "Waiting for MQTT data..."

    # Line chart for lat, lng, battery
    fig = go.Figure()
    fig.add_trace(
        go.Scatter(
            y=list(latitudes),
            x=list(timestamps),
            mode="lines",
            name="Latitude",
            line=dict(dash="dash", color="red"),
        )
    )
    fig.add_trace(
        go.Scatter(
            y=list(longitudes),
            x=list(timestamps),
            mode="lines",
            name="Longitude",
            line=dict(dash="dash", color="blue"),
        )
    )
    fig.add_trace(
        go.Scatter(
            y=list(batteries),
            x=list(timestamps),
            mode="lines",
            name="Battery (%)",
            line=dict(dash="dash", color="green"),
        )
    )
    fig.update_layout(
        title="Latitude, Longitude, and Battery (Live)",
        xaxis_title="Time",
        yaxis_title="Value",
        template="plotly_dark",
    )

    # Display text data
    info_text = (
        f"ID: {latest_data['id']} | "
        f"Date: {latest_data['date']} | "
        f"Time: {latest_data['time']}"
    )

    return fig, info_text


if __name__ == "__main__":
    app.run(host="0.0.0.0", port=8050, debug=True)
