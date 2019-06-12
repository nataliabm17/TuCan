import serial
import struct
import time

# ser.open()
# ser.flushInput()
# ser.flushOutput()


flagCharacter = 44
entra = True


v_team_id = 0; # 0 team_id
v_time = 0  # 1 mission time
v_packet_count = 0  # 2
v_altitude = 0  # 3
v_pressure = 0  # 4
v_temp = 0  # 5
v_voltage = 0  # 6
v_gps_time_hour = 0  # 7
v_gps_time_minute = 0  # 8
v_gps_time_seconds = 0  # 9
v_gps_latitude = 0  # 10
v_gps_longitude = 0  # 11
v_gps_altitude = 0  # 12
v_gps_sats = 0  # 13
v_pitch = 0  # 14
v_roll = 0  # 15
v_blade_spin_rate = 0  # 16
v_software_state = 0  # 17

v = [v_time, v_team_id, v_packet_count, v_altitude, v_pressure, v_temp, v_voltage, v_gps_time_hour,
     v_gps_time_minute, v_gps_time_seconds, v_gps_latitude, v_gps_longitude, v_gps_altitude,
     v_gps_sats, v_pitch, v_roll, v_blade_spin_rate, v_software_state]


while True:
    ser = serial.Serial('COM7', timeout=None, baudrate=115200, xonxoff=False, rtscts=False, dsrdtr=False)
    time.sleep(0.1)
    data_raw = ser.read(4)
    [x] = struct.unpack('f', data_raw)

    if round(x) == 1000000:

        for i in range(0, 7):
            data_raw = ser.read(1)
            data_raw = ser.read(4)
            [x] = struct.unpack('f', data_raw)
            v[i] = x

        data_raw = ser.read(1)
        for i in range(7, 10):
            data_raw = ser.read(4)
            [x] = struct.unpack('f', data_raw)
            v[i] = x

        for i in range(10, 18):
            data_raw = ser.read(1)
            data_raw = ser.read(4)
            [x] = struct.unpack('f', data_raw)
            v[i] = x

        # data_raw = ser.read(1)

    print(round(v[0]), ",", round(v[1]), ",", round(v[2]), ",", round(v[3], 1), ",", round(v[4], 1), ",",
          round(v[5], 2), ",", round(v[6], 2), ",", round(v[7]), ":", round(v[8]), ":", round(v[9]), ",",
          round(v[10], 4), ",", round(v[11], 4), ",", round(v[12], 1), ",", round(v[13]), ",", round(v[14]), ",",
          round(v[15]), ",", round(v[16]), ",", round(v[17]))

    if v[2] > 5.0 and v[2] < 6.0:
        ser.write(bytes(flagCharacter))
        # time.sleep(1.8)

    ser.close()

    # if x == 1000000:
    #  b = ser.read(1)  # time
    #  data_raw = ser.read(4)
    #  [x] = struct.unpack('f', data_raw)
    #  print(round(x))

    #  b = ser.read(1)
    #  print(',')

    #  data_raw = ser.read(4)  # packet_count
    #  [x] = struct.unpack('f', data_raw)
    #  print(round(x))

    # if a == 255:
    # data_raw = ser.read(1)
    # [x] = struct.unpack('f', data_raw)
    # print(round(x))
    # [x] = struct.unpack('f', data_raw)
    # a = int.from_bytes(data_raw, byteorder='little')



