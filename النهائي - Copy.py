import paho.mqtt.client as mqtt
from flask import Flask, render_template, request, redirect, url_for,Response
from flask_sqlalchemy import SQLAlchemy
from datetime import datetime
import qrcode
import os
import cv2
from pyzbar.pyzbar import decode

app = Flask(__name__)
app.config["SQLALCHEMY_DATABASE_URI"] = "sqlite:///project.db"
db = SQLAlchemy(app)
# MQTT broker address
broker_address = "192.168.0.88"

mqtt_topic = "parking/status"
mqtt_topic1 = "parking/told"
mqtt_topic2 = "parking/status1"
mqtt_topic3 = "parking/told1"

# Dictionary to store parking space status
parking_status = {}
parking_status1 = {}


class Booking(db.Model):
    id = db.Column(db.Integer, primary_key=True)
    name = db.Column(db.String(100), nullable=False)
    car_number = db.Column(db.String(100), nullable=False)
    car_type = db.Column(db.String(50), nullable=False)
    phone_number = db.Column(db.String(13), nullable=False)
    date = db.Column(db.Date, nullable=False)
    time = db.Column(db.Time, nullable=False)
    datetime = db.Column(db.DateTime, nullable=False, default=datetime.utcnow)
    space_id = db.Column(db.Integer, nullable=False)
    position = db.Column(db.Integer, nullable=False)


    def __repr__(self):
        return (f"Booking('{self.name}', '{self.car_number}', '{self.car_type}', '{self.phone_number}',"
                f" '{self.date}', '{self.time}', '{self.datetime}', '{self.space_id}',{self.position})")

def read_qr_code(frame):
    decoded_objects = decode(frame)
    for obj in decoded_objects:
        data = obj.data.decode("utf-8")
        return data

def on_message(client, userdata, msg):
    global parking_status
    #print(mqtt_topic + " " + str(msg.payload))
    #print("shutup")
    payload = msg.payload.decode()
    parking_space_id, status = payload.split(":")
    if parking_space_id.isdigit():
        parking_status[int(parking_space_id)] = status
    else:
        print("Invalid parking space ID received:", parking_space_id)

def on_message1(client2, userdata, msg1):
    global parking_status1
    #print(mqtt_topic + " " + str(msg.payload))
    #print("shutup")
    payload1 = msg1.payload.decode()
    parking_space_id1, status1 = payload1.split(":")
    if parking_space_id1.isdigit():
        parking_status1[int(parking_space_id1)] = status1

    else:
        print("Invalid parking space ID received:", parking_space_id1)


def on_connect(client1, userdata, flags, rc):
    print("Connected with result code " + str(rc))
    client1.subscribe(mqtt_topic)
    client1 = mqtt.Client()
    client1.on_connect = on_connect

def on_connect1(client2, userdata, flags, rc):
    print("Connected with result code " + str(rc))
    client2.subscribe(mqtt_topic2)
    client2 = mqtt.Client()
    client2.on_connect = on_connect

    # Publish MQTT message after successful connection
    #x = client.publish(mqtt_topic, f"{space_id}:booked")
    #print(x)

@app.route('/')
def home():
    image_path = '/static/img/1.png'
    return render_template('layout.html',image_path=image_path)

@app.route('/dashboard')
def dashboard():
    image_path = '/static/img/1.png'
    return render_template('dashboard.html', parking_status=parking_status,image_path=image_path)

@app.route('/dashboard2')
def dashboard2():
    image_path = '/static/img/1.png'
    return render_template('dashboard2.html', parking_status1=parking_status1,image_path=image_path)

@app.route('/AMA', methods=['GET', 'POST'])
def AMA():
    image_path = '/static/img/1.png'
    client = mqtt.Client()  # تعريف المتغير client هنا
    if request.method == 'POST':
        name = request.form['name']
        car_number = request.form['car_number']
        car_type = request.form['car_type']
        phone_number = request.form['phone_number']
        date_str = request.form['date']
        time_str = request.form['time']
        date = datetime.strptime(date_str, '%Y-%m-%d').date()
        time = datetime.strptime(time_str, '%H:%M').time()
        space_id = request.form['space_id']
        position = request.form['position']

        def is_space_booked(space_id, date, time):
            return Booking.query.filter_by(space_id=space_id, date=date, time=time, position=position).first() is not None

        # Check if space is already booked
        booking_status = is_space_booked(space_id, date, time)

        if booking_status:
            print(f"Parking space {space_id} is already booked.")
           # return "This parking space is already booked."
            return render_template('booked.html')

        else:
            print(f"Parking space {space_id} is now booked.")

            # Add new booking to database
            new_booking = Booking(name=name, car_number=car_number, car_type=car_type, phone_number=phone_number,
                                  date=date, time=time, space_id=space_id, position=position)
            db.session.add(new_booking)
            db.session.commit()

            # Create QR code for booking details
            booking_details = (f"Name: {name}, Car Number: {car_number}, Car Type: {car_type}, Phone Number: {phone_number},"
                               f" Date: {date}, Time: {time}, Space ID: {space_id},position: {position}")
            qr = qrcode.QRCode(version=1, box_size=10, border=5)
            qr.add_data(booking_details)
            qr.make(fit=True)
            qr_image = qr.make_image(fill_color="black", back_color="white")

            # Create a directory if it doesn't exist
            if not os.path.exists("static"):
                os.makedirs("static")

            # Save QR code image
            qr_image_path = os.path.join(app.root_path, "static", f"booking_{new_booking.id}_qr.png")
            qr_image.save(qr_image_path)



            # Redirect to success page after successful booking
            return redirect(url_for('success', booking_id=new_booking.id))

    return render_template('AMA.html',image_path=image_path)




@app.route('/booking_success/<int:booking_id>')
def success(booking_id):
    # Get the booking information from the database
    booking = Booking.query.get_or_404(booking_id)

    # Generate the URL for the QR code image
   # image_path_qr = '/static/f"booking_{booking.id}_qr.png"'
    image_path_qr = url_for('static', filename=f"booking_{booking.id}_qr.png")

#    return render_template('success.html', booking=booking, qr_image_path=qr_image_path)
    return render_template('success.html', booking=booking, image_path_qr=image_path_qr)

def generate_frames(camera_index):
    camera = cv2.VideoCapture(camera_index)

    while True:
        success, frame = camera.read()

        if not success:
            break
        else:
            data = read_qr_code(frame)
            if data:
                status = check_database_for_data(data)
                if status is not None:
                    yield (b'--frame\r\n'
                           b'Content-Type: text/plain\r\n\r\n' + status.encode() + b'\r\n')

            ret, buffer = cv2.imencode('.jpg', cv2.flip(frame, 1))
            frame = buffer.tobytes()
            yield (b'--frame\r\n'
                   b'Content-Type: image/jpeg\r\n\r\n' + frame + b'\r\n')

    camera.release()

def print_booking_data():
    with app.app_context():
        try:
            bookings = Booking.query.all()
            if bookings:
                for booking in bookings:
                    print(booking)
            else:
                print("No bookings found in the database")
        except Exception as e:
            print("Error:", e)

def check_database_for_data(data):
    with app.app_context():
        try:
            print("Received data:", data)
            name = data.split(',')[0].split(': ')[1]  # استخراج اسم الحجز من البيانات

            # استعلام قاعدة البيانات للبحث عن الحجز المطابق
            booking = Booking.query.filter_by(name=name).first()

            if booking:
                print("Booking found:", booking)

                # التحقق مما إذا كانت قيمة position تساوي 0 أو 1 وطباعة النتيجة المناسبة
                if booking.position == 0:
                    print("جيد")
                    # Publish MQTT message after booking
                    message_content = "open gate1"
                    client.connect(broker_address, 1883, 60)
                    publish_result = client.publish(mqtt_topic1, message_content)
                    print("Message published successfully")
                    print(mqtt_topic1, "Message content:", message_content)
                elif booking.position == 1:
                    print("ممتاز")
                    # Publish MQTT message to topic4 when booking is excellent
                    message_content = "open gate"
                    client.connect(broker_address, 1883, 60)
                    publish_result = client.publish(mqtt_topic3, message_content)
                    print("Message published successfully to", mqtt_topic3)
            else:
                print("No booking found with this data")
        except ValueError:
            print("Error parsing data from QR code")


@app.route('/camera')
def camera():
    return render_template('camera.html')

@app.route('/camera1')
def camera1():
    return render_template('camera1.html')

@app.route('/video_feed')
def video_feed():
    return Response(generate_frames(0), mimetype='multipart/x-mixed-replace; boundary=frame')

@app.route('/video_feed2')
def video_feed1():
    return Response(generate_frames(1), mimetype='multipart/x-mixed-replace; boundary=frame')

if __name__ == '__main__':
    with app.app_context():
        db.create_all()
        client = mqtt.Client()
        client.on_connect = on_connect
        client.on_message = on_message
        client.connect(broker_address, 1883, 60)
        client.loop_start()  # بدء دورة MQTT client
        client2 = mqtt.Client()
        client2.on_connect = on_connect1
        client2.on_message = on_message1
        client2.connect(broker_address, 1883, 60)
        client2.loop_start()

    app.run(debug=True, host='0.0.0.0', port=80)