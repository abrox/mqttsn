#!/usr/bin/env python      
import Tkinter as tk     
import Queue
import logging
import communication as cm
import crc8
import argparse

appName= "JukIOT Base configurator"

class Application(tk.Frame):             
    def __init__(self, cfg, master=None):
        tk.Frame.__init__(self, master)
 
        self.queue  = Queue.Queue( maxsize=20 )# Just prevent's increase infinity... 
        self.logger = logging.getLogger('BatteryMonitor')
        self.port   = cm.ArduinoIf( cfg, self)
        self.master = master

        master.protocol("WM_DELETE_WINDOW", self.on_closing)        
        self.grid()                       
        self.createWidgets()
        self.port.start()
        self.periodicCall()

    def createWidgets(self):
        #################################################
        row = tk.Frame(self)
        lab = tk.Label(row, width=15, text="NodeId:\n1-255", anchor='w')
        vcmd = (self.register(self.validateNodeId),
                '%d', '%i', '%P', '%s', '%S', '%v', '%V', '%W')
        ent = tk.Entry(row, validate = 'key', validatecommand = vcmd)
        row.pack(side=tk.TOP, fill=tk.X, padx=5, pady=5)
        lab.pack(side=tk.LEFT)
        ent.pack(side=tk.RIGHT, expand=tk.YES, fill=tk.X)
        self.NodeId=ent
        #################################################
        row = tk.Frame(self)
        lab = tk.Label(row, width=15, text="Dev name:\nmax len 10", anchor='w')
        vcmd = (self.register(self.validateName),
                '%d', '%i', '%P', '%s', '%S', '%v', '%V', '%W')
        ent = tk.Entry(row, validate = 'key', validatecommand = vcmd)
        row.pack(side=tk.TOP, fill=tk.X, padx=5, pady=5)
        lab.pack(side=tk.LEFT)
        ent.pack(side=tk.RIGHT, expand=tk.YES, fill=tk.X)
        self.devName = ent
        ####################################################
        row = tk.Frame(self)
        lab = tk.Label(row, width=15, text="From dev:", anchor='w')
        t=    tk.Text(row, height = 5)
        row.pack(side=tk.TOP, fill=tk.X, padx=5, pady=5)
        lab.pack(side=tk.LEFT)
        t.pack(side=tk.RIGHT, expand=tk.YES, fill=tk.X)
        self.txtView=t
        ####################################################
        row = tk.Frame(self)
        b = tk.Button(row, text="Close", command=self.on_closing)
        b2 = tk.Button(row, text="Write Config", command=self.callback)
        lab = tk.Label(row, width=15, text="Cmd:", anchor='w')
        row.pack(side=tk.TOP, fill=tk.X, padx=5, pady=5)
        lab.pack(side=tk.LEFT)
        b.pack(side=tk.RIGHT)
        b2.pack(side=tk.LEFT)
        ####################################################
   
    def validateNodeId(self, action, index, value_if_allowed,
                       prior_value, text, validation_type, trigger_type, widget_name):
        if len(value_if_allowed) == 0:
           return True;
        if text in '0123456789':
            try:
                float(value_if_allowed)
                if( int(value_if_allowed) > 255):
                   return False
                return True
            except ValueError:
                return False
        else:
            return False

    def validateName(self, action, index, value_if_allowed,
                     prior_value, text, validation_type, trigger_type, widget_name):
        if len(value_if_allowed) <= 10:
           return True;
        else:
            return False

    def callback(self):
        id   = int(self.NodeId.get())
        name = self.devName.get()
        #|0|msglen|nodeid|name n bytes|CRC|
        msgLen  = len(name)
        if( msgLen == 0):
            return

        msgLen += 3
        data    = chr(msgLen)
        data   += chr(id)+name 
        crc     = crc8.crc8(data)
        data   += chr(crc)
        print data
        print "MsgLen: %d and CRC:%x"%(msgLen,crc)
        self.port.send(chr(0))#begin of msg
        self.port.send(data)

    def periodicCall(self):
        """
        Check every 100 ms if there is something new in the queue.
        """
        self.processIncoming()
        self.master.after(100, self.periodicCall)
                
    def processIncoming(self):
        try:
           data = self.queue.get(block=False)
           if len(data):
              print data
              self.txtView.delete(1.0, tk.END)
              self.txtView.insert(tk.END,data)
        except Queue.Empty:
           pass

    def putQ(self, msg):
        '''Callback function handling incoming messages'''
        rc=True
        try:
            self.queue.put(msg,False)
        except Queue.Full,e:
            self.logger.error('Queue overflow: '+ str(e))
            self.alive = False #No reason to continue 
            rc= False
        return rc

    def on_closing(self):
        print "On closing...."
        self.port.alive = False
        #give some time for seriar port thread exit...
        self.port.join(3.0)
        self.master.destroy()
        

###################################################################
def createLogger():
    # create logger
    logger = logging.getLogger(appName)
    logger.setLevel(logging.DEBUG)
    # create console handler and set level to debug
    ch = logging.StreamHandler()
    ch.setLevel(logging.DEBUG)
    # create formatter
    formatter = logging.Formatter('%(asctime)s - %(name)s - %(levelname)s - %(message)s')
    # add formatter to ch
    ch.setFormatter(formatter)
    # add ch to logger
    logger.addHandler(ch)

def handleCmdLineArgs():
    parser = argparse.ArgumentParser()
    parser.add_argument('--port','-p',help='Serial port arduino connected',default='/dev/ttyUSB0')
    parser.add_argument('--speed','-s',help='Serial port speed', choices=['9600', '19200', '38400', '57600','115200'],default='9600' )
    return parser.parse_args()

def main( args ):
    myCfg={'port':args.port,'speed':args.speed,'name':appName}
    createLogger()
    window = tk.Tk()
    app = Application(cfg=myCfg, master= window)                       
    window.title(appName)  
    window.mainloop()             

if __name__ == '__main__':
    main(handleCmdLineArgs())
