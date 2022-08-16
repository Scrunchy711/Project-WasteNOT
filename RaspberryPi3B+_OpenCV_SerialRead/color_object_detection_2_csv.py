from imutils.video import VideoStream
import imutils
import numpy as np
import argparse
import time
import cv2
import RPi.GPIO as GPIO
import serial
from datetime import datetime
import csv

num1 = 1
num2 = 1

# This code has been commentted out as implementation of it has not been stable. Main function is to store the serial inputs of 2 Arduinos
# (Sensor Platform and Flake collection system) containing the weight and colour of PLA object and PLA flakes respectively for future
# work in implementation of UI which requires such backend data. 

# ser1=serial.Serial("/dev/ttyACM0", 9600,timeout=1)
# ser1.baudrate=9600
# ser1.reset_input_buffer()
# ser2=serial.Serial("/dev/ttyACM0",9600,timeout=1)
# ser2.baudrate=9600
# ser2.reset_input_buffer()

person_id = "1004283" 

GPIO.setmode(GPIO.BOARD)
GPIO.setwarnings(False)
GPIO.setup(12, GPIO.OUT)
GPIO.setup(3, GPIO.OUT)
GPIO.setup(5, GPIO.IN, pull_up_down=GPIO.PUD_DOWN)
p = GPIO.PWM(12,50)

print("[INFO] waiting for camera to warmup... \n")
webcam = VideoStream(src=1).start()
picam = VideoStream(usePiCamera=True).start()
time.sleep(2.0)

# color here is red
redLower_pi = np.array([169, 100, 100], dtype=np.uint8)
redUpper_pi = np.array([189,255, 255], dtype=np.uint8)

# color here is for webcam red
redLower_web = np.array([0, 50, 50], dtype=np.uint8)
redUpper_web = np.array([10, 255, 255], dtype=np.uint8)

blueLower = np.array([80, 100, 100], dtype=np.uint8)
blueUpper = np.array([130,255, 255], dtype=np.uint8)

yellowLower = np.array([18, 100, 100], dtype=np.uint8)
yellowUpper = np.array([38,255, 255], dtype=np.uint8)

whiteLower = np.array([0, 0, 180], dtype=np.uint8)
whiteUpper = np.array([255,255, 255], dtype=np.uint8)
hsv_array = [[],[]]
cnts_array = [[[],[],[],[]],[[],[],[],[]]]
mask_array = [[[],[],[],[]],[[],[],[],[]]]
# on_state = False

with open('example1.csv','w',newline='') as csvfile1, open('example2.csv','w',newline='') as csvfile2:
    c1 = csv.writer(csvfile1,delimiter=' ',quotechar='|',quoting=csv.QUOTE_MINIMAL)
    c1.writerow(['Log','ID','Input Weight','Colour','Timestamp'])
    
    c2 = csv.writer(csvfile2,delimiter=' ',quotechar='|',quoting=csv.QUOTE_MINIMAL)
    c2.writerow(['Log','Output Weight','Colour','Timestamp'])

while True:
#     if ser1.in_waiting > 0:
#         line = ser1.readline().decode('utf-8',errors='replace').rstrip()
#         print(line)
#         if line[0] == "H":
#             input_line = line.split(",")
#             print(input_line)
#             with open('example1.csv','a',newline='') as csvfile1:
#                 c1 = csv.writer(csvfile1,delimiter=' ',quotechar='|',quoting=csv.QUOTE_MINIMAL)
#                 c1.writerow([num1]+[person_id]+[input_line[2]]+[input_line[3]]+[datetime.today().strftime('%Y-%m-%d %H:%M:%S')])
#                 num1 = num1 + 1
#     else:
#         print("never recieve from Master Arduino \n")
        
#     if ser2.in_waiting > 0:
#         line = ser2.readline().decode('utf-8',errors='replace').rstrip()
#         print(line)
#         if line[0] == "H":
#             input_line = line.split(",")
#             print(input_line)
#             with open('example2.csv','a',newline='') as csvfile2:
#                 c2 = csv.writer(csvfile2,delimiter=' ',quotechar='|',quoting=csv.QUOTE_MINIMAL)
#                 c2.writerow([num2]+[input_line[2]]+[input_line[3]]+[datetime.today().strftime('%Y-%m-%d %H:%M:%S')])
#                 num2 = num2 + 1
#     else:
#         print("never recieve from StrShrdCol Arduino \n")
        
    piFrame = picam.read()
    webFrame = webcam.read()
    frames = [piFrame, webFrame]
    # convert bgr frames to hsv
#     print(list(enumerate(frames)))
    for i,frame in enumerate(frames):
        print("color convert for frame no. " + str(i))
#         frame = imutils.resize(frame, width=500)
        hsv = cv2.cvtColor(frame, cv2.COLOR_BGR2HSV)
        hsv_array[i] = hsv
#     print(list(enumerate(hsv_array)))

    for i,hsvFrame in enumerate(hsv_array):
        print(i)
        if i == 0:
            redMask = cv2.inRange(hsvFrame, redLower_pi, redUpper_pi)
        else:
            redMask = cv2.inRange(hsvFrame, redLower_web, redLower_web)
        redMask = cv2.erode(redMask, None, iterations=2)
        redMask = cv2.dilate(redMask, None, iterations=2)
#         res_red = cv2.bitwise_and(piFrame, piFrame,
#                                   mask = redMask)
        redCnts = cv2.findContours(redMask.copy(), cv2.RETR_EXTERNAL,
                                   cv2.CHAIN_APPROX_SIMPLE)
        redCnts = imutils.grab_contours(redCnts)
#         print(redCnts)
        cnts_array[i][0] = redCnts
        mask_array[i][0] = redMask
    
        blueMask = cv2.inRange(hsvFrame, blueLower, blueUpper)
        blueMask = cv2.erode(blueMask, None, iterations=2)
        blueMask = cv2.dilate(blueMask, None, iterations=2)
        blueCnts = cv2.findContours(blueMask.copy(), cv2.RETR_EXTERNAL,
                                    cv2.CHAIN_APPROX_SIMPLE)
        blueCnts = imutils.grab_contours(blueCnts)
#         print(blueCnts)
        cnts_array[i][1] = blueCnts
        mask_array[i][1] = blueMask

        yellowMask = cv2.inRange(hsvFrame, yellowLower, yellowUpper)
        yellowMask = cv2.erode(yellowMask, None, iterations=2)
        yellowMask = cv2.dilate(yellowMask, None, iterations=2)
        yellowCnts = cv2.findContours(yellowMask.copy(), cv2.RETR_EXTERNAL,
                                      cv2.CHAIN_APPROX_SIMPLE)
        yellowCnts = imutils.grab_contours(yellowCnts)
        cnts_array[i][2] = yellowCnts
        mask_array[i][2] = yellowMask
    
        whiteMask = cv2.inRange(hsvFrame, whiteLower, whiteUpper)
        whiteMask = cv2.erode(whiteMask, None, iterations=2)
        whiteMask = cv2.dilate(whiteMask, None, iterations=2)
        whiteCnts = cv2.findContours(whiteMask.copy(), cv2.RETR_EXTERNAL,
                                     cv2.CHAIN_APPROX_SIMPLE)
        whiteCnts = imutils.grab_contours(whiteCnts)
#         print(whiteCnts)
        cnts_array[i][3] = whiteCnts
        mask_array[i][3] = whiteMask
    
    center = None
    
    # color sensor
    if len(cnts_array[0][0]) > 0:
        redCnts_pi = cnts_array[0][0]
        c = max(redCnts_pi, key=cv2.contourArea)
        ((x,y),radius) = cv2.minEnclosingCircle(c)
        M = cv2.moments(c)
        center = (int(M["m10"] / M["m00"]),int(M["m01"]/M["m00"]))
        if radius > 10:
            cv2.circle(frames[0], (int(x), int(y)), int(radius),
                           (0, 0, 255), 2)
            cv2.circle(frames[0], center, 5, (0, 0, 255), -1)
            print(" I see Red \n")
            p.start(20)
            
    elif  len(cnts_array[0][1]) > 0:
        blueCnts_pi = cnts_array[0][1]
        c = max(blueCnts_pi, key=cv2.contourArea)
        ((x,y),radius) = cv2.minEnclosingCircle(c)
        M = cv2.moments(c)
        center = (int(M["m10"] / M["m00"]),int(M["m01"]/M["m00"]))
        if radius > 10:
            cv2.circle(frames[0], (int(x), int(y)), int(radius),
                           (255, 0, 0), 2)
            cv2.circle(frames[0], center, 5, (255, 0, 0), -1)
            print(" I see BLue \n")
            p.start(40)
            
    elif len(cnts_array[0][2]) > 0:
        print("I see yellow \n" )
        yellowCnts_pi = cnts_array[0][2]
        print(yellowCnts_pi)
        c = max(yellowCnts_pi, key=cv2.contourArea)
        ((x,y),radius) = cv2.minEnclosingCircle(c)
        M = cv2.moments(c)
        center = (int(M["m10"] / M["m00"]),int(M["m01"]/M["m00"]))
        if radius > 10:
            cv2.circle(frames[0], (int(x), int(y)), int(radius),
                           (0, 255, 255), 2)
            cv2.circle(frames[0], center, 5, (0, 255, 255), -1)
            p.start(60)
            
    elif  len(cnts_array[0][3]) > 0:
        whiteCnts_pi = cnts_array[0][3]
        c = max(whiteCnts_pi, key=cv2.contourArea)
        ((x,y),radius) = cv2.minEnclosingCircle(c)
        M = cv2.moments(c)
        center = (int(M["m10"] / M["m00"]),int(M["m01"]/M["m00"]))
        if radius > 10:
            cv2.circle(frames[0], (int(x), int(y)), int(radius),
                           (255, 255, 255), 2)
            cv2.circle(frames[0], center, 5, (255, 255, 255), -1) 
            print(" I see White \n")
            p.start(80)
            
    else:
        p.stop()
    
    #observing if shredder has finished
    if GPIO.input(5):
        print("Shredder Webcam is on! \n")
        if len(cnts_array[1][0]) > 0:
            redCnts_web = cnts_array[1][0]
            c = max(redCnts_web, key=cv2.contourArea)
            ((x,y),radius) = cv2.minEnclosingCircle(c)
            M = cv2.moments(c)
            center = (int(M["m10"] / M["m00"]),int(M["m01"]/M["m00"]))
            if radius > 10:
                cv2.circle(frames[1], (int(x), int(y)), int(radius),
                           (0, 0, 255), 2)
                cv2.circle(frames[1], center, 5, (0, 0, 255), -1)
                GPIO.output(3, GPIO.HIGH)
                print(" I see Red in the shredder \n")
                
        elif len(cnts_array[1][1]) > 0:
            blueCnts_web = cnts_array[1][1]
            c = max(blueCnts_web, key=cv2.contourArea)
            ((x,y),radius) = cv2.minEnclosingCircle(c)
            M = cv2.moments(c)
            center = (int(M["m10"] / M["m00"]),int(M["m01"]/M["m00"]))
            if radius > 10:
                cv2.circle(frames[1], (int(x), int(y)), int(radius),
                           (255, 0, 0), 2)
                cv2.circle(frames[1], center, 5, (255, 0, 0), -1)
                GPIO.output(3, GPIO.HIGH)
                print(" I see Blue in the shredder \n")
                
        elif len(cnts_array[1][2]) > 0:
            yellowCnts_web = cnts_array[1][2]
            c = max(yellowCnts_web, key=cv2.contourArea)
            ((x,y),radius) = cv2.minEnclosingCircle(c)
            M = cv2.moments(c)
            center = (int(M["m10"] / M["m00"]),int(M["m01"]/M["m00"]))
            if radius > 10:
                cv2.circle(frames[1], (int(x), int(y)), int(radius),
                           (0, 255, 255), 2)
                cv2.circle(frames[1], center, 5, (0, 255, 255), -1)
                GPIO.output(3, GPIO.HIGH)
                print(" I see Yellow in the shredder \n")
            
        elif  len(cnts_array[1][3]) > 0:
            whiteCnts_web = cnts_array[1][3]
            c = max(whiteCnts_web, key=cv2.contourArea)
            ((x,y),radius) = cv2.minEnclosingCircle(c)
            M = cv2.moments(c)
            center = (int(M["m10"] / M["m00"]),int(M["m01"]/M["m00"]))
            if radius > 10:
                cv2.circle(frames[1], (int(x), int(y)), int(radius),
                           (255, 255, 255), 2)
                cv2.circle(frames[1], center, 5, (255, 255, 255), -1) 
                GPIO.output(3, GPIO.HIGH)
                print(" I see White in the shredder \n")
        else:
            print("no more items in the shredder \n")
            GPIO.output(3, GPIO.LOW)
            # insert a feedback signal for no receive of item
        
            
#     print(mask_array[0][0])
#     piRedMask = mask_array[0][0]
#     res_red = cv2.bitwise_and(frames[0], frames[0],
#                               mask = piRedMask)

    cv2.imshow("piFrame", frames[0])
    cv2.imshow("mask", mask_array[0][0])
    cv2.imshow("webFrame", frames[1])
#     cv2.imshow("yellowmask", mask_array[0][2])
    
#     cv2.imshow("piRed", res_red)
    key = cv2.waitKey(1) & 0xFF
    
    if key == ord("q"):
        break
    
print("\n Exiting Program and cleanup stuff \n")
cv2.destroyAllWindows()
webcam.stop()
picam.stop()
GPIO.cleanup()



    
    