#!/usr/bin/env python
'''
The MIT License (MIT)
Copyright (c) 2014  Jukka-Pekka Sarjanen
Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:
The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.
THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
'''
import serial
import threading
import time
import logging


NOT_CONNECTED = 1
CONNECTED     = 2
NOT_CREATED   = 3

class ArduinoIf (threading.Thread):
    ''' RS232 interface to actual monitor.
        resposible to connect and re connect if cable is removed 
        or power is swtiched off from arduino'''
    def __init__(self, cfg, callBackIf):
        threading.Thread.__init__(self)
        self.cb        = callBackIf
        self.cfg       = cfg
        self.portState = None
        self.ser       = None
        self.alive     = True
        
        self.logger    = logging.getLogger(cfg['name'])
        self.createAndSetPortState(cfg)

    def createAndSetPortState(self, cfg):
        ''' Create and open port.also update port state.'''
        try:
            self.ser = port = serial.Serial( port=cfg['port'],
                                  baudrate=cfg['speed'], 
                                  dsrdtr=True, # this and self.ser.setDTR(False) Prevents arduino boot when connect
                                  timeout=0.5)
            port.setDTR(False)
            self.portState = CONNECTED
        except serial.SerialException,e:
            self.portState = NOT_CREATED
            self.logger.warning('Serial Exp: '+ str(e)) 
 
    def run(self):
        while self.alive:
            try:
                if( self.portState is CONNECTED ):
                    msg = self.ser.read(100)
                    #print msg
                    #if '\n' in msg:
                        #msg = msg.rstrip('\r\n')
                        #If we are not able to handle incomming messages -> it's time to die...
                    if( self.alive):
                        self.alive = self.cb.putQ(msg)
                elif( self.portState is NOT_CONNECTED ):
                    if( self.ser.isOpen() ):
                        self.ser.close() 
                    else:
                        self.logger.debug('Trying open port')     
                        self.ser.open()#If cable is missing this will throw
                        self.logger.debug('Port Open') 
                        self.portState = CONNECTED
                elif( self.portState is NOT_CREATED ):
                    self.createAndSetPortState(self.cfg)
                    print self.alive
                    if not self.ser:
                         time.sleep(2) 
            except serial.SerialException,e:
                self.logger.error('Serial Exp: '+ str(e)) 
                time.sleep(2)
                self.portState = NOT_CONNECTED
        self.logger.info('Serial Thread say bye bye')
        
    def send(self,data):
        try:
            if self.portState is CONNECTED:
                print self.ser.write(data)
        except serial.SerialException,e:
            self.logger.error('Writefails'+ str(e))
            self.portState = NOT_CONNECTED
           
    def __del__(self):
        self.alive = False
        if self.ser:
            self.ser.close()
