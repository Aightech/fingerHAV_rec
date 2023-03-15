import cv2
import numpy as np

# Load image
image = cv2.imread("rec_29_11_22/1t/1t_0_c0.png")

# select 4 points and then crop the image
pts = []
def select_point(event, x, y, flags, param):
    global pts
    if event == cv2.EVENT_LBUTTONDOWN:
        pts.append((x, y))
        # draw the point
        cv2.circle(image, (x, y), 5, (0, 0, 255), -1)
        cv2.imshow("image", image)
        if len(pts) == 4:
            print(pts)
            cv2.destroyAllWindows()

cv2.namedWindow("image")
cv2.setMouseCallback("image", select_point)
cv2.imshow("image", image)
cv2.waitKey(0)

# crop the image
pts = np.array(pts)
rect = cv2.boundingRect(pts)
x, y, w, h = rect
croped = image[y:y+h, x:x+w].copy()

# save the image
cv2.imwrite("croped.png", croped)

# show the image
cv2.imshow("croped", croped)

# wait for a key to exit
cv2.waitKey(0)
