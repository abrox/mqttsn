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
    return 0;
}
int Nrf24Net::send(const uint8_t * buffer,uint16_t buffSize,NetworkAddr *addr){
    int rc = _mesh.write(buffer, 'M', buffSize);

    // If a write fails, check connectivity to the mesh network
    if(!rc){
        if ( ! _mesh.checkConnection() ) {
            //refresh the network address
            Serial.println("Renewing Address");
            _mesh.renewAddress();
        }
    }
    return rc;
}
int Nrf24Net::recv(uint8_t * buffer,uint16_t buffSize){
    RF24NetworkHeader header;
    _network.read(header, buffer, sizeof(buffSize));
    return 0;
}

NetworkAddr * Nrf24Net::getLastRecvAddr(){
    return NULL;
}
#endif
