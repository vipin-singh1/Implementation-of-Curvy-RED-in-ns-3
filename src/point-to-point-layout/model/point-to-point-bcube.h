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

// Define an object to create a BCube topology.

#ifndef POINT_TO_POINT_BCUBE_HELPER_H
#define POINT_TO_POINT_BCUBE_HELPER_H

#include <vector>

#include "internet-stack-helper.h"
#include "point-to-point-helper.h"
#include "ipv4-address-helper.h"
#include "ipv6-address-helper.h"
#include "ipv4-interface-container.h"
#include "ipv6-interface-container.h"
#include "net-device-container.h"

namespace ns3 {

/**
 * \ingroup point-to-point-layout
 *
 * \brief A helper to make it easier to create a BCube topology
 * with p2p links
 */
class PointToPointBCubeHelper
{
public:
  /**
   * Create a PointToPointBCubeHelper in order to easily create
   * BCube topologies using p2p links
   *
   * \param nLevels total number of levels in BCube
   *
   * \param nServers total number of servers in one BCube
   *
   * \param pointToPoint the PointToPointHelper which is used
   *                     to connect all of the nodes together
   *                     in the BCube
   */
  PointToPointBCubeHelper (uint32_t nLevels,
                           uint32_t nServers,
                           PointToPointHelper pointToPoint);

  ~PointToPointBCubeHelper ();

  /**
   * \param row the row address of the desired switch
   *
   * \param col the column address of the desired switch
   *
   * \returns a pointer to the switch specified by the
   *          (row, col) address
   */
  Ptr<Node> GetSwitchNode (uint32_t row, uint32_t col) const;

  /**
   * \param col the column address of the desired server
   *
   * \returns a pointer to the server specified by the
   *          column address
   */
  Ptr<Node> GetServerNode (uint32_t col) const;

  /**
   * This returns an IPv4 address of the switch specified by
   * the (row, col) address. Technically, a switch will have
   * multiple interfaces in the BCube; therefore, it also has
   * multiple IPv4 addresses. This method only returns one of
   * the addresses. The address being returned belongs to an
   * interface which connects the lowest index server to this
   * switch.
   *
   * \param row the row address of the desired switch
   *
   * \param col the column address of the desired switch
   *
   * \returns Ipv4Address of one of the interfaces of the switch
   *          specified by the (row, col) address
   */
  Ipv4Address GetSwitchIpv4Address (uint32_t row, uint32_t col) const;

  /**
   * This returns an IPv6 address at the switch specified by
   * the (row, col) address. Technically, a switch will have
   * multiple interfaces in the BCube; therefore, it also has
   * multiple IPv6 addresses. This method only returns one of
   * the addresses. The address being returned belongs to an
   * interface which connects the lowest index server to this
   * switch.
   *
   * \param row the row address of the desired switch
   *
   * \param col the column address of the desired switch
   *
   * \returns Ipv6Address of one of the interfaces of the switch
   *          specified by the (row, col) address
   */
  Ipv6Address GetSwitchIpv6Address (uint32_t row, uint32_t col) const;

  /**
   * This returns an IPv4 address at the server specified by
   * the col address. Technically, a server will have multiple
   * interfaces in the BCube; therefore, it also has multiple
   * IPv4 addresses. This method only returns one of the addresses.
   * The address being returned belongs to an interface which
   * connects the lowest level switch to this server.
   *
   * \param col the column address of the desired server
   *
   * \returns Ipv4Address of one of the interfaces of the server
   *          specified by the column address
   */
  Ipv4Address GetServerIpv4Address (uint32_t col) const;

  /**
   * This returns an IPv6 address at the server specified by
   * the col address. Technically, a server will have multiple
   * interfaces in the BCube; therefore, it also has multiple
   * IPv6 addresses. This method only returns one of the addresses.
   * The address being returned belongs to an interface which
   * connects the lowest level switch to this server.
   *
   * \param col the column address of the desired server
   *
   * \returns Ipv6Address of one of the interfaces of the server
   *          specified by the column address
   */
  Ipv6Address GetServerIpv6Address (uint32_t col) const;

  /**
   * \param stack an InternetStackHelper which is used to install
   *              on every node in the BCube
   */
  void InstallStack (InternetStackHelper stack);

  /**
   * Assigns IPv4 addresses to all the interfaces of switch
   *
   * \param network an IPv4 address representing the network portion
   *                of the IPv4 address
   *
   * \param mask the mask length
   */
  void AssignIpv4Addresses (Ipv4Address network, Ipv4Mask mask);

  /**
   * Assigns IPv6 addresses to all the interfaces of the switch
   *
   * \param network an IPv6 address representing the network portion
   *                of the IPv6 address
   *
   * \param prefix the prefix length
   */
  void AssignIpv6Addresses (Ipv6Address network, Ipv6Prefix prefix);

  /**
   * Sets up the node canvas locations for every node in the BCube.
   * This is needed for use with the animation interface
   *
   * \param ulx upper left x value
   * \param uly upper left y value
   * \param lrx lower right x value
   * \param lry lower right y value
   */
  void BoundingBox (double ulx, double uly, double lrx, double lry);

private:
  uint32_t m_numLevels;                                         //!< number of levels (k)
  uint32_t m_numServers;                                        //!< number of servers (n)
  std::vector<NetDeviceContainer> m_levelSwitchDevices;         //!< Net Device container for servers and switches
  std::vector<Ipv4InterfaceContainer> m_switchInterfaces;       //!< IPv4 interfaces of switch
  Ipv4InterfaceContainer m_serverInterfaces;                    //!< IPv4 interfaces of server
  std::vector<Ipv6InterfaceContainer> m_switchInterfaces6;      //!< IPv6 interfaces of switch
  Ipv6InterfaceContainer m_serverInterfaces6;                   //!< IPv6 interfaces of server
  NodeContainer m_switches;                                     //!< all the switches in the BCube
  NodeContainer m_servers;                                      //!< all the servers in the BCube
};

} // namespace ns3

#endif /* POINT_TO_POINT_BCUBE_HELPER_H */
