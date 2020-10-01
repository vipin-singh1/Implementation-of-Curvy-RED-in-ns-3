/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2017 NITK Surathkal
 *
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
 *
 * Authors: Shravya K.S. <shravya.ks0@gmail.com>
 *
 */

// Implement an object to create a BCube topology.

#include "ns3/string.h"
#include "ns3/vector.h"
#include "ns3/log.h"

#include "ns3/point-to-point-bcube.h"
#include "ns3/point-to-point-helper.h"
#include "ns3/point-to-point-net-device.h"
#include "ns3/internet-stack-helper.h"
#include "ns3/ipv4-address-generator.h"
#include "ns3/ipv6-address-generator.h"
#include "ns3/constant-position-mobility-model.h"

namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("PointToPointBCubeHelper");

PointToPointBCubeHelper::PointToPointBCubeHelper (uint32_t nLevels,
                                                  uint32_t nServers,
                                                  PointToPointHelper p2pHelper)
  : m_numLevels (nLevels),
    m_numServers (nServers)
{
  // Bounds check
  if (nServers < 1)
    {
      NS_FATAL_ERROR ("Insufficient number of servers for BCube.");
    }
  uint32_t numLevelSwitches = pow (nServers, nLevels);
  m_levelSwitchDevices.resize ((nLevels + 1) * numLevelSwitches);
  m_switchInterfaces.resize ((nLevels + 1) * numLevelSwitches);

  // Number of servers = pow (nServers, nLevels + 1)
  //                   = nServers * pow (nServers, nLevels)
  //                   = nServers * numLevelSwitches
  m_servers.Create (nServers * numLevelSwitches);

  // Number of switches = (nLevels + 1) * pow (nServers, nLevels)
  //                    = (nLevels + 1) * numLevelSwitches
  m_switches.Create ((nLevels + 1) * numLevelSwitches);

  InternetStackHelper stack;

  uint32_t switchColId;
  // Configure the levels in BCube topology
  for (uint32_t level = 0; level < nLevels + 1; level++)
    {
      switchColId = 0;
      uint32_t val1 = pow (nServers, level);
      uint32_t val2 = val1 * nServers;
      // Configure the position of switches at each level of the topology
      for (uint32_t switches = 0; switches < numLevelSwitches; switches++)
        {
          uint32_t serverIndex = switches % val1 + switches / val1 * val2;
          // Connect nServers to every switch
          for (uint32_t servers = serverIndex; servers < (serverIndex + val2); servers += val1)
            {
              NetDeviceContainer nd = p2pHelper.Install (m_servers.Get (servers),
                                                         m_switches.Get (level * numLevelSwitches + switchColId));
              m_levelSwitchDevices[level * numLevelSwitches + switchColId].Add (nd.Get (0));
              m_levelSwitchDevices[level * numLevelSwitches + switchColId].Add (nd.Get (1));
            }
          switchColId += 1;
        }
    }
}

PointToPointBCubeHelper::~PointToPointBCubeHelper ()
{
}

void
PointToPointBCubeHelper::InstallStack (InternetStackHelper stack)
{
  stack.Install (m_servers);
  stack.Install (m_switches);
}

void
PointToPointBCubeHelper::BoundingBox (double ulx, double uly,
                                      double lrx, double lry)
{
  NS_LOG_FUNCTION (this << ulx << uly << lrx << lry);
  double xDist;
  double yDist;
  if (lrx > ulx)
    {
      xDist = lrx - ulx;
    }
  else
    {
      xDist = ulx - lrx;
    }
  if (lry > uly)
    {
      yDist = lry - uly;
    }
  else
    {
      yDist = uly - lry;
    }

  uint32_t val = pow (m_numServers, m_numLevels);
  uint32_t numServers = val * m_numServers;
  double xServerAdder = xDist / numServers;
  double xSwitchAdder = m_numServers * xServerAdder;
  double yAdder = yDist / (m_numLevels + 2);

  // Place the servers
  double xLoc = 0.0;
  double yLoc = yDist / 2;
  for (uint32_t i = 0; i < numServers; ++i)
    {
      Ptr<Node> node = m_servers.Get (i);
      Ptr<ConstantPositionMobilityModel> loc = node->GetObject<ConstantPositionMobilityModel> ();
      if (loc == 0)
        {
          loc = CreateObject<ConstantPositionMobilityModel> ();
          node->AggregateObject (loc);
        }
      Vector locVec (xLoc, yLoc, 0);
      loc->SetPosition (locVec);
      xLoc += 2 * xServerAdder;
    }

  yLoc -= yAdder;

  // Place the switches
  for (uint32_t i = 0; i < m_numLevels + 1; ++i)
    {
      if (m_numServers % 2 == 0)
        {
          xLoc = xSwitchAdder / 2 + xServerAdder;
        }
      else
        {
          xLoc = xSwitchAdder / 2 + xServerAdder / 2;
        }
      for (uint32_t j = 0; j < val; ++j)
        {
          Ptr<Node> node = m_switches.Get (i * val + j);
          Ptr<ConstantPositionMobilityModel> loc = node->GetObject<ConstantPositionMobilityModel> ();
          if (loc == 0)
            {
              loc = CreateObject<ConstantPositionMobilityModel> ();
              node->AggregateObject (loc);
            }
          Vector locVec (xLoc, yLoc, 0);
          loc->SetPosition (locVec);

          xLoc += 2 * xSwitchAdder;
        }
      yLoc -= yAdder;
    }
}

void
PointToPointBCubeHelper::AssignIpv4Addresses (Ipv4Address network, Ipv4Mask mask)
{
  NS_LOG_FUNCTION (this << network << mask);
  Ipv4AddressGenerator::Init (network, mask);
  Ipv4Address v4network;
  Ipv4AddressHelper addrHelper;

  for (uint32_t i = 0; i < m_levelSwitchDevices.size (); ++i)
    {
      v4network = Ipv4AddressGenerator::NextNetwork (mask);
      addrHelper.SetBase (v4network, mask);
      for (uint32_t j = 0; j < m_levelSwitchDevices[i].GetN (); j += 2)
        {
          Ipv4InterfaceContainer ic = addrHelper.Assign (m_levelSwitchDevices[i].Get (j));
          m_serverInterfaces.Add (ic);
          ic = addrHelper.Assign (m_levelSwitchDevices[i].Get (j + 1));
          m_switchInterfaces[i].Add (ic);
        }
    }
}

void
PointToPointBCubeHelper::AssignIpv6Addresses (Ipv6Address addrBase, Ipv6Prefix prefix)
{
  NS_LOG_FUNCTION (this << addrBase << prefix);
  Ipv6AddressGenerator::Init (addrBase, prefix);
  Ipv6Address v6network;
  Ipv6AddressHelper addrHelper;

  for (uint32_t i = 0; i < m_levelSwitchDevices.size (); ++i)
    {
      v6network = Ipv6AddressGenerator::NextNetwork (prefix);
      addrHelper.SetBase (v6network, prefix);
      for (uint32_t j = 0; j < m_levelSwitchDevices[i].GetN (); j += 2)
        {
          Ipv6InterfaceContainer ic = addrHelper.Assign (m_levelSwitchDevices[i].Get (j));
          m_serverInterfaces6.Add (ic);
          ic = addrHelper.Assign (m_levelSwitchDevices[i].Get (j + 1));
          m_switchInterfaces6[i].Add (ic);
        }
    }
}

Ipv4Address
PointToPointBCubeHelper::GetServerIpv4Address (uint32_t i) const
{
  NS_LOG_FUNCTION (this << i);
  return m_serverInterfaces.GetAddress (i);
}

Ipv4Address
PointToPointBCubeHelper::GetSwitchIpv4Address (uint32_t i, uint32_t j) const
{
  NS_LOG_FUNCTION (this << i << j);
  return m_switchInterfaces[i].GetAddress (j);
}

Ipv6Address
PointToPointBCubeHelper::GetServerIpv6Address (uint32_t i) const
{
  NS_LOG_FUNCTION (this << i);
  return m_serverInterfaces6.GetAddress (i, 1);
}

Ipv6Address
PointToPointBCubeHelper::GetSwitchIpv6Address (uint32_t i, uint32_t j) const
{
  NS_LOG_FUNCTION (this << i << j);
  return m_switchInterfaces6[i].GetAddress (j, 1);
}

Ptr<Node>
PointToPointBCubeHelper::GetServerNode (uint32_t i) const
{
  NS_LOG_FUNCTION (this << i);
  return m_servers.Get (i);
}

Ptr<Node>
PointToPointBCubeHelper::GetSwitchNode (uint32_t i, uint32_t j) const
{
  NS_LOG_FUNCTION (this << i << j);
  uint32_t numLevelSwitches = pow (m_numServers, m_numLevels);
  return m_switches.Get (i * numLevelSwitches + j);
}

} // namespace ns3
