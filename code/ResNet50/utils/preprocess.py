import cv2 
import numpy as np
import os

def Rubost_OpticalFlow_KL(video):
    frames = 0
    video = cv2.VideoCapture(video) 
    if not video.isOpened():
        print(f"Runtime Error: Cannot rea video {video}")
        exit()

    iVideoTime = int(video.get(cv2.CAP_PROP_FRAME_COUNT))
    #print(iVideoTime)

    _, frame = video.read()
    prev_frame = cv2.cvtColor(frame, cv2.COLOR_BGR2GRAY)
    prev_points = cv2.goodFeaturesToTrack(prev_frame, 500, 0.10, 20)

    dst = np.zeros(frame.shape, dtype=frame.dtype)

    while True:
        ret, frame = video.read()
        if not ret:
            break

        gray_frame = cv2.cvtColor(frame, cv2.COLOR_BGR2GRAY)
        next_points, status, err = cv2.calcOpticalFlowPyrLK(prev_frame, gray_frame, prev_points, None)

        for i in range(len(prev_points)):
            if status[i]:
                point_dist = np.sqrt((prev_points[i][0][0] - next_points[i][0][0]) ** 2 + (prev_points[i][0][1] - next_points[i][0][1]) ** 2)

                if point_dist < 50:
                    color_R = int((frames * 255) / iVideoTime)
                    color_G = int((frames * 255) / iVideoTime)
                    cv2.line(frame, (int(prev_points[i][0][0]), int(prev_points[i][0][1])), (int(next_points[i][0][0]), int(next_points[i][0][1])), (color_R, color_G, 255), 2)
                    cv2.line(dst, (int(prev_points[i][0][0]), int(prev_points[i][0][1])), (int(next_points[i][0][0]), int(next_points[i][0][1])), (color_R, color_G, 255), 2)
                    cv2.circle(frame, (int(next_points[i][0][0]), int(next_points[i][0][1])), 1, (0, 255, 0), -1)

        #cv2.imshow("Optical Flow", dst)
        #cv2.imshow("Optical Flowframe", frame)
        #key = cv2.waitKey(0)
        #if key == 27:
        #    break

        prev_frame = gray_frame
        prev_points = next_points
        frames += 1

    #cv2.imshow("Optical Flow", dst)
    #cv2.imshow("Optical Flowframe", frame)
    #cv2.waitKey(0)
    video.release()
    cv2.destroyAllWindows()
    return dst