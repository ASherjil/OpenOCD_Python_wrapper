import multiprocessing as mp
from subprocess import Popen
import time
from OpenOCD import OpenOCD

class Device:
    def __init__(self, id: str, name: str, uid: str, port: str, TelNetPort: int, TCLPort: int, startCMD: str) -> None:
        self.id = id
        self.name = name
        self.uid = uid
        self.port = port
        self.TelNetPort = TelNetPort
        self.TCLPort = TCLPort
        self.serialOutput = mp.Queue()
        
        print(f"Starting OpenOCD for device {name} with UID {uid} on TelNet port {TelNetPort} and TCL port {TCLPort}")
        print(f"OpenOCD start command: openocd -c adapter serial {uid} -c telnet_port {TelNetPort} -c tcl_port {TCLPort} -f {startCMD}")
        
        self.OpenOCDProcess = Popen(["openocd", "-c", f"adapter serial {uid}", "-c", f"telnet_port {TelNetPort}", 
                                     "-c", f"tcl_port {TCLPort}", "-f", startCMD])
        
        print(f"{name}: Created OpenOCD Process, PID: {self.OpenOCDProcess.pid}")
        
        if self.OpenOCDProcess.poll() is not None:
            error_msg = f'Could not open OpenOCD session for {name} on TelNet Port: {TelNetPort}'
            print(error_msg)
            raise Exception(error_msg)  # Use a specific exception as needed
        
        time.sleep(0.1)  # Give OpenOCD time to start up
        
        self.OpenOCDSession = OpenOCD(Port=TelNetPort)# connect to port 
        _read_out = self.OpenOCDSession.Reset(Init=True)
        print(f'Readout from reset: {_read_out}')
        #_read_out = self.OpenOCDSession.Exec("target", "current")
        #print(f'Readout from target current: {_read_out}')
        self.time_commands()

    def time_commands(self):
        start_time = time.time()#---------Start timing now

        for _ in range(1):
            _read_out1 = self.OpenOCDSession.Exec("bp", "0x8003010", "2", "hw")
            _read_out2 = self.OpenOCDSession.Exec("rbp", "0x8003010")
            #_read_out1 = self.OpenOCDSession.Exec("flash write_image", "erase", "/home/realtime/Documents/Part_D_Modules/Individual_Project/EmbeddedNNAccuracyTest/elfFiles/Mag_Acc_Dataset_EKF_M7_Cksm.elf", "0x0", "elf")
            #_read_out2 = self.OpenOCDSession.Exec("flash verify_image", "/home/realtime/Documents/Part_D_Modules/Individual_Project/EmbeddedNNAccuracyTest/elfFiles/Mag_Acc_Dataset_EKF_M7_Cksm.elf", "0x0", "elf")

        end_time = time.time()#------------End timing now
        print(f'Readout1: {_read_out1}')
        print(f'Readout2: {_read_out2}')
        time_taken_ms = (end_time - start_time) * 1000  
        print(f"Time taken for tests: {time_taken_ms:.2f} milliseconds")

    def disconnect(self):
        print(f"{self.name}: Disconnecting OpenOCD session...")
        # Here, implement the logic to disconnect from the OpenOCD session if necessary
        # For example, sending a command to OpenOCD to halt the target and exit

        # Terminate the OpenOCD process
        if self.OpenOCDProcess:
            self.OpenOCDProcess.terminate()
            self.OpenOCDProcess.wait()  # Ensure the process has indeed terminated
            print(f"{self.name}: OpenOCD process terminated.")


def main():
    # Define your device's parameters
    device_id = "1"
    device_name = "ttyACM3"
    device_uid = "066EFF574887534867011040"
    device_port = "USB"
    telnet_port = 4440
    tcl_port = 6660
    start_cmd = "./board/st_nucleo_f7.cfg"

    # Device initialization code...
    try:
        device = Device(id=device_id, name=device_name, uid=device_uid, port=device_port, 
                        TelNetPort=telnet_port, TCLPort=tcl_port, startCMD=start_cmd)
        print(f"{device_name} initialized successfully with tests completed")
        
        # Placeholder for user input to continue/exit; could be replaced with signal handling in a real-world application
        #input("Press Enter to disconnect and exit the application...")
        device.disconnect() # safely disconnect and shutdown 

    except Exception as e:
        print(f"Failed to initialize {device_name}: {e}")
    finally:
        print("Application exiting...")

if __name__ == "__main__":
    main()