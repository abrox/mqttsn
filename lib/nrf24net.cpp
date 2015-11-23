#include "mqttclientdefs.h"
#include "nrf24net.h"

#ifdef ARDUINO

#define nodeID 1

Nrf24Net::Nrf24Net():_radio(7, 8),_network(_radio),_mesh(_radio, _network)

{
    _mesh.setNodeID(nodeID);

}
int16_t Nrf24Net::initilize(){

   // Connect to the mesh
   _mesh.begin();
   _mesh.update();
      return 0;

}
int Nrf24Net::send(const uint8_t * buffer,uint16_t buffSize,NetworkAddr *addr){
    bool   dataSent;
    uint8_t reTry=3;

    _mesh.update();

    // If a write fails, check connectivity to the mesh network
    while( (dataSent = _mesh.write(buffer, 'Q', buffSize) != true && reTry-- )){
        if ( ! _mesh.checkConnection() ) {
            //refresh the network address
            Serial.println("Renewing Address");
            _mesh.renewAddress();
        }
        break;
    }

    return dataSent?buffSize:0;
}
int Nrf24Net::recv(uint8_t * buffer,uint16_t buffSize){
    RF24NetworkHeader header;
    uint16_t count;

   _mesh.update();

   if( _network.available() )
       return _network.read(header, buffer,buffSize);
   else
       return 0;

}

NetworkAddr * Nrf24Net::getLastRecvAddr(){
    return NULL;
}
#endif
