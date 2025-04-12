// /*  p2pool‑simulation.cc   –  Summer‑of‑Bitcoin competency test
//  *
//  *  ns‑3.35 ‑ g++17
//  *  -----------------------------------------------------------
//  *  mesh / random P2P network of ≥10 nodes, “shares” as messages
//  *  each node generates and forwards shares to all peers
//  *  -----------------------------------------------------------
//  */

// #include "ns3/core-module.h"
// #include "ns3/network-module.h"
// #include "ns3/internet-module.h"
// #include "ns3/point-to-point-module.h"
// #include "ns3/applications-module.h"

// using namespace ns3;
// NS_LOG_COMPONENT_DEFINE ("P2PoolSimulation");

// /* ---------- helpers ---------------------------------------------------- */

// static void
// ReceivePacket (Ptr<Socket> socket)      // called whenever a share arrives
// {
//   Address from;
//   Ptr<Packet> pkt;
//   while ((pkt = socket->RecvFrom (from)))
//     {
//       uint32_t len = pkt->GetSize ();
//       std::vector<uint8_t> buf (len);
//       pkt->CopyData (buf.data (), len);
//       std::string msg (reinterpret_cast<char *> (buf.data ()), len);

//       std::cout << Simulator::Now ().GetSeconds () << " s  "
//                 << socket->GetNode ()->GetId () << " recv  \""
//                 << msg << "\"  from "
//                 << InetSocketAddress::ConvertFrom (from).GetIpv4 () << std::endl;
//     }
// }

// static void
// SendShareOnce (Ptr<Socket>   sock,
//                Ipv4Address   peer,
//                uint16_t      port,
//                std::string   payload)
// {
//   Ptr<Packet> pkt = Create<Packet> (reinterpret_cast<const uint8_t *> (payload.c_str ()),
//                                     payload.size ());
//   sock->SendTo (pkt, 0, InetSocketAddress (peer, port));

//   std::cout << Simulator::Now ().GetSeconds () << " s  "
//             << sock->GetNode ()->GetId () << " send  \"" << payload
//             << "\"  to " << peer << std::endl;
// }

// /* schedule the next send every PERIOD seconds -------------------------- */
// static void
// SchedulePeriodicShare (Ptr<Socket> sock,
//                        Ipv4Address peer,
//                        uint16_t    port,
//                        std::string payload,
//                        Time        period)
// {
//   SendShareOnce (sock, peer, port, payload);
//   Simulator::Schedule (period,
//                        &SchedulePeriodicShare,
//                        sock, peer, port, payload, period);
// }

// /* ---------- main ------------------------------------------------------- */

// int
// main (int argc, char *argv[])
// {
//   uint32_t nNodes = 10;         // default ≥10
//   uint16_t port   = 9999;
//   Time     period = Seconds (2.0);

//   CommandLine cmd;
//   cmd.AddValue ("nNodes", "number of P2P nodes", nNodes);
//   cmd.Parse (argc, argv);

//   /* 1. create & stack -------------------------------------------------- */
//   NodeContainer nodes;
//   nodes.Create (nNodes);
//   InternetStackHelper internet;
//   internet.Install (nodes);

//   /* 2. point‑to‑point helper ------------------------------------------ */
//   PointToPointHelper p2p;
//   p2p.SetDeviceAttribute   ("DataRate", StringValue ("5Mbps"));
//   p2p.SetChannelAttribute  ("Delay",    StringValue ("2ms"));

//   Ipv4AddressHelper ip;
//   uint32_t subnet = 1;
//   std::vector<Ipv4InterfaceContainer> ifacePairs;

//   /* 3. build a *full mesh* -------------------------------------------- */
//   for (uint32_t i = 0; i < nNodes; ++i)
//     for (uint32_t j = i + 1; j < nNodes; ++j)
//       {
//         NetDeviceContainer dev = p2p.Install (NodeContainer (nodes.Get (i),
//                                                              nodes.Get (j)));
//         std::ostringstream net;
//         net << "10." << subnet++ << ".0.0";
//         ip.SetBase (net.str ().c_str (), "255.255.255.0");
//         ifacePairs.push_back (ip.Assign (dev));
//       }

//   /* 4. sockets: one Rx per node, one Tx per peer ----------------------- */
//   for (uint32_t i = 0; i < nNodes; ++i)
//     {
//       /* receiver socket */
//       Ptr<Socket> rx = Socket::CreateSocket (nodes.Get (i),
//                                              UdpSocketFactory::GetTypeId ());
//       rx->Bind (InetSocketAddress (Ipv4Address::GetAny (), port));
//       rx->SetRecvCallback (MakeCallback (&ReceivePacket));
//     }

//   /* map node‑id → its first IP address (index 1,0 = point‑to‑point) */
//   std::vector<Ipv4Address> nodeIp (nNodes);
//   for (uint32_t i = 0; i < nNodes; ++i)
//     nodeIp[i] = nodes.Get (i)->GetObject<Ipv4> ()->GetAddress (1, 0).GetLocal ();

//   /* schedule periodic share traffic ----------------------------------- */
//   for (uint32_t i = 0; i < nNodes; ++i)
//     {
//       Ptr<Socket> tx = Socket::CreateSocket (nodes.Get (i),
//                                              UdpSocketFactory::GetTypeId ());

//       std::string payload = "share_from_" + std::to_string (i);

//       for (uint32_t j = 0; j < nNodes; ++j)
//         if (i != j)
//           {
//             Time offset = MilliSeconds (100 * (j + 1));   // small staggering
//             Simulator::Schedule (Seconds (1.0) + offset,
//                                  &SchedulePeriodicShare,
//                                  tx, nodeIp[j], port, payload, period);
//           }
//     }

//   Simulator::Stop (Seconds (15.0));
//   Simulator::Run ();
//   Simulator::Destroy ();
//   return 0;
// }












#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/applications-module.h"
#include "ns3/netanim-module.h"

using namespace ns3;
NS_LOG_COMPONENT_DEFINE ("P2PoolSimulation");

static void ReceivePacket (Ptr<Socket> socket)
{
  Address from;
  Ptr<Packet> pkt;
  while ((pkt = socket->RecvFrom (from)))
    {
      uint32_t len = pkt->GetSize ();
      std::vector<uint8_t> buf (len);
      pkt->CopyData (buf.data (), len);
      std::string msg (reinterpret_cast<char*> (buf.data ()), len);

      std::cout << Simulator::Now ().GetSeconds () << " s  "
                << socket->GetNode ()->GetId () << " recv  \"" << msg
                << "\"  from " << InetSocketAddress::ConvertFrom (from).GetIpv4 ()
                << std::endl;
    }
}

static void SendShareOnce (Ptr<Socket> sock, Ipv4Address peer, uint16_t port, std::string payload)
{
  Ptr<Packet> pkt = Create<Packet> ((uint8_t*) payload.c_str (), payload.length ());
  sock->SendTo (pkt, 0, InetSocketAddress (peer, port));

  std::cout << Simulator::Now ().GetSeconds () << " s  "
            << sock->GetNode ()->GetId () << " send  \"" << payload
            << "\"  to " << peer << std::endl;
}

static void SchedulePeriodicShare (Ptr<Socket> sock, Ipv4Address peer, uint16_t port, std::string payload, Time period)
{
  SendShareOnce (sock, peer, port, payload);
  Simulator::Schedule (period, &SchedulePeriodicShare, sock, peer, port, payload, period);
}

int main (int argc, char *argv[])
{
  uint32_t nNodes = 10;
  uint16_t port = 9999;
  Time period = Seconds (2.0);

  CommandLine cmd;
  cmd.AddValue ("nNodes", "Number of P2P nodes", nNodes);
  cmd.Parse (argc, argv);

  NodeContainer nodes;
  nodes.Create (nNodes);
  InternetStackHelper internet;
  internet.Install (nodes);

  PointToPointHelper p2p;
  p2p.SetDeviceAttribute ("DataRate", StringValue ("5Mbps"));
  p2p.SetChannelAttribute ("Delay", StringValue ("2ms"));

  Ipv4AddressHelper ip;
  uint32_t subnet = 1;
  std::vector<Ipv4InterfaceContainer> ifacePairs;

  for (uint32_t i = 0; i < nNodes; ++i)
    for (uint32_t j = i + 1; j < nNodes; ++j)
      {
        NetDeviceContainer dev = p2p.Install (NodeContainer (nodes.Get (i), nodes.Get (j)));
        std::ostringstream net;
        net << "10." << subnet++ << ".0.0";
        ip.SetBase (net.str ().c_str (), "255.255.255.0");
        Ipv4InterfaceContainer iface = ip.Assign (dev);
        ifacePairs.push_back (iface);
      }

  p2p.EnablePcapAll ("p2pool");

  AnimationInterface anim ("p2pool.xml");
  anim.SetMaxPktsPerTraceFile (100000);

  std::vector<Ipv4Address> nodeIp (nNodes);
  for (uint32_t i = 0; i < nNodes; ++i)
    nodeIp[i] = nodes.Get (i)->GetObject<Ipv4> ()->GetAddress (1, 0).GetLocal ();

  for (uint32_t i = 0; i < nNodes; ++i)
    {
      Ptr<Socket> rx = Socket::CreateSocket (nodes.Get (i), UdpSocketFactory::GetTypeId ());
      rx->Bind (InetSocketAddress (Ipv4Address::GetAny (), port));
      rx->SetRecvCallback (MakeCallback (&ReceivePacket));
    }

  for (uint32_t i = 0; i < nNodes; ++i)
    {
      Ptr<Socket> tx = Socket::CreateSocket (nodes.Get (i), UdpSocketFactory::GetTypeId ());
      std::string payload = "share_from_" + std::to_string (i);

      for (uint32_t j = 0; j < nNodes; ++j)
        {
          if (i == j) continue;
          Time offset = MilliSeconds (100 * (j + 1));
          Simulator::Schedule (Seconds (1.0) + offset,
                               &SchedulePeriodicShare,
                               tx, nodeIp[j], port, payload, period);
        }
    }

  Simulator::Stop (Seconds (15.0));
  Simulator::Run ();
  Simulator::Destroy ();
  return 0;
}
