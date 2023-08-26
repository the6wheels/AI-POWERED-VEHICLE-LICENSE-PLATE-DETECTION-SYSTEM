from flask import Flask, request, jsonify

app = Flask(__name__)

@app.route('/', methods=['GET'])
def index():
    return app.send_static_file('index.html')

@app.route('/api/license_plate', methods=['POST'])
def process_license_plate():
    # Extract the license plate number from the request
    plate_number = request.form.get('plate_number')

    # Perform database lookup and retrieve driver information based on the plate number
    # Replace this code with your database query logic

    driver_info = {
        'name': 'John Doe',
        'license_number': 'ABC123',
        'vehicle_model': 'Toyota Camry',
        'color': 'Red'
    }

    return jsonify(driver_info)

if __name__ == '__main__':
    app.run(host='0.0.0.0', port=5000)
