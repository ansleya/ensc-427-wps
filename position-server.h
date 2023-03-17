/*
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation;
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#ifndef POSITION_SERVER_H
#define POSITION_SERVER_H

#include "ns3/core-module.h"
#include "ns3/internet-module.h"
#include "ns3/network-module.h"

namespace ns3
{

class Application;

/**
 * Tutorial - a simple Application sending packets.
 */
class PositionServer : public Application
{
  public:
    PositionServer();
    ~PositionServer() override;

    /**
     * Register this type.
     * \return The TypeId.
     */
    static TypeId GetTypeId();

    /**
     * Setup the socket.
     * \param socket The socket.
     * \param address The destination address.
     * \param packetSize The packet size to transmit.
     * \param nPackets The number of packets to transmit.
     * \param dataRate the datarate to use.
     */
    void Setup(Ptr<Socket> socket,
               Address address,
               uint32_t packetSize,
               uint32_t nPackets,
               DataRate dataRate);
             
    void sendLocation(); // AP node master sends the estimated Location back to the phone user client.     
    void getRTT(); // AP Minion Routers retrieve the RTT from the client
    void forwardRTTMaster(); //AP minions send their RTTs to master AP 
    void computeLocation(); //AP master computes the location based on received RTTs
  private:
    void StartApplication() override;
    void StopApplication() override;

    /// Schedule a new transmission.
    void ScheduleTx();
    /// Send a packet.
    void SendPacket();
    
    uint32_t estLocation[2]; // 0: x, 1: y
    
/* we may need these, but idk yet
    Ptr<Socket> m_socket;   //!< The tranmission socket.
    Address m_peer;         //!< The destination address.
    uint32_t m_packetSize;  //!< The packet size.
    uint32_t m_nPackets;    //!< The number of pacts to send.
    DataRate m_dataRate;    //!< The datarate to use.
    EventId m_sendEvent;    //!< Send event.
    bool m_running;         //!< True if the application is running.
    uint32_t m_packetsSent; //!< The number of pacts sent.*/
};

} // namespace ns3

#endif /* POSITION_SERVER_H */
