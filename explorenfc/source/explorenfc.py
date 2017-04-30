import nxppy
import time

mifare = nxppy.Mifare()

try:
    uid = mifare.select()
    print(uid)
except nxppy.SelectError:
    pass 
time.sleep(0.5)
