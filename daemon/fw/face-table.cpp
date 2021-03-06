/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/**
 * Copyright (c) 2014,  Regents of the University of California,
 *                      Arizona Board of Regents,
 *                      Colorado State University,
 *                      University Pierre & Marie Curie, Sorbonne University,
 *                      Washington University in St. Louis,
 *                      Beijing Institute of Technology,
 *                      The University of Memphis
 *
 * This file is part of NFD (Named Data Networking Forwarding Daemon).
 * See AUTHORS.md for complete list of NFD authors and contributors.
 *
 * NFD is free software: you can redistribute it and/or modify it under the terms
 * of the GNU General Public License as published by the Free Software Foundation,
 * either version 3 of the License, or (at your option) any later version.
 *
 * NFD is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY;
 * without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
 * PURPOSE.  See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * NFD, e.g., in COPYING.md file.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "face-table.hpp"
#include "forwarder.hpp"
#include "core/logger.hpp"

namespace nfd {

NFD_LOG_INIT("FaceTable");

FaceTable::FaceTable(Forwarder& forwarder)
  : m_forwarder(forwarder)
  , m_lastFaceId(FACEID_RESERVED_MAX)
{
}

FaceTable::~FaceTable()
{

}

void
FaceTable::add(shared_ptr<Face> face)
{
  if (face->getId() != INVALID_FACEID && m_faces.count(face->getId()) > 0) {
    NFD_LOG_WARN("Trying to add existing face id=" << face->getId() << " to the face table");
    return;
  }

  FaceId faceId = ++m_lastFaceId;
  BOOST_ASSERT(faceId > FACEID_RESERVED_MAX);
  this->addImpl(face, faceId);
}

void
FaceTable::addReserved(shared_ptr<Face> face, FaceId faceId)
{
  BOOST_ASSERT(face->getId() == INVALID_FACEID);
  BOOST_ASSERT(m_faces.count(face->getId()) == 0);
  BOOST_ASSERT(faceId <= FACEID_RESERVED_MAX);
  this->addImpl(face, faceId);
}

void
FaceTable::addImpl(shared_ptr<Face> face, FaceId faceId)
{
  face->setId(faceId);
  m_faces[faceId] = face;
  NFD_LOG_INFO("Added face id=" << faceId << " remote=" << face->getRemoteUri()
                                          << " local=" << face->getLocalUri());

  face->onReceiveInterest += bind(&Forwarder::onInterest,
                                  &m_forwarder, ref(*face), _1);
  face->onReceiveData     += bind(&Forwarder::onData,
                                  &m_forwarder, ref(*face), _1);
  face->onFail            += bind(&FaceTable::remove,
                                  this, face);

  this->onAdd(face);
}

void
FaceTable::remove(shared_ptr<Face> face)
{
  this->onRemove(face);

  FaceId faceId = face->getId();
  m_faces.erase(faceId);
  face->setId(INVALID_FACEID);
  NFD_LOG_INFO("Removed face id=" << faceId << " remote=" << face->getRemoteUri() <<
                                                 " local=" << face->getLocalUri());

  // XXX This clears all subscriptions, because EventEmitter
  //     does not support only removing Forwarder's subscription
  face->onReceiveInterest.clear();
  face->onReceiveData    .clear();
  face->onSendInterest   .clear();
  face->onSendData       .clear();
  // don't clear onFail because other functions may need to execute

  m_forwarder.getFib().removeNextHopFromAllEntries(face);
}



} // namespace nfd
