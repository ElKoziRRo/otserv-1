//////////////////////////////////////////////////////////////////////
// OpenTibia - an opensource roleplaying game
//////////////////////////////////////////////////////////////////////
//
//////////////////////////////////////////////////////////////////////
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software Foundation,
// Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
//////////////////////////////////////////////////////////////////////

#ifndef __NETWORK_MESSAGE_H__
#define __NETWORK_MESSAGE_H__


#include "definitions.h"
#include "otsystem.h"
#include "const80.h"
#include "item.h"

class Creature;
class Player;
class Position;
class RSA;

class NetworkMessage
{
public:
	// constructor/destructor
	NetworkMessage(){
		m_encryptionEnabled = false;
		m_keyset = false;
		m_RSA = NULL;
		Reset();
	}
	~NetworkMessage(){};

	// resets the internal buffer to an empty message
	void Reset(){
		m_MsgSize = 0;
		m_ReadPos = 4;
	}

	// socket functions
	bool ReadFromSocket(SOCKET socket);
	bool WriteToSocket(SOCKET socket);

	// simply read functions for incoming message
	uint8_t  GetByte(){return m_MsgBuf[m_ReadPos++];}
	uint16_t GetU16(){
		uint16_t v = *(uint16_t*)(m_MsgBuf + m_ReadPos);
		m_ReadPos += 2;
		return v;
	}
	uint16_t GetSpriteId(){
		return GetU16();
	}
	uint32_t GetU32(){
		uint32_t v = *(uint32_t*)(m_MsgBuf + m_ReadPos);
		m_ReadPos += 4;
		return v;
	}
	std::string GetString();
	std::string GetRaw();
	Position GetPosition();

	void setEncryptionState(bool state){m_encryptionEnabled = state;}
	void setEncryptionKey(const uint32_t* key){
		memcpy(m_key, key, 16);
		m_keyset = true;
	}

	// skips count unknown/unused bytes in an incoming message
	void SkipBytes(int count){m_ReadPos += count;}

	// simply write functions for outgoing message
	void AddByte(uint8_t  value){
		if(!canAdd(1))
			return;
		m_MsgBuf[m_ReadPos++] = value;
		m_MsgSize++;
	}
	void AddU16(uint16_t value){
		if(!canAdd(2))
			return;
		*(uint16_t*)(m_MsgBuf + m_ReadPos) = value;
		m_ReadPos += 2; m_MsgSize += 2;
	}
	void AddU32(uint32_t value){
		if(!canAdd(4))
			return;
		*(uint32_t*)(m_MsgBuf + m_ReadPos) = value;
		m_ReadPos += 4; m_MsgSize += 4;
	}
	void AddBytes(const char* bytes, uint32_t size);

	void AddString(const std::string &value){AddString(value.c_str());}
	void AddString(const char* value);


	// write functions for complex types
	void AddPosition(const Position &pos);
	void AddItem(uint16_t id, uint8_t count);
	void AddItem(const Item *item);
	void AddItemId(const Item *item){
		const ItemType &it = Item::items[item->getID()];
		AddU16(it.clientId);
	}
	void AddItemId(uint16_t itemId){
		const ItemType &it = Item::items[itemId];
		AddU16(it.clientId);
	}
	void AddCreature(const Creature *creature, bool known, unsigned int remove);

  	int getMessageLength(){
		return m_MsgSize;
	}
	
	bool empty() const { return m_MsgSize == 0;}
	void JoinMessages(NetworkMessage &add);

	bool RSA_decrypt();

	void setRSAInstance(RSA* rsa){m_RSA = rsa;}

protected:
	inline bool canAdd(int size)
	{
		return (size + m_ReadPos < NETWORKMESSAGE_MAXSIZE - 16);
  };
  	
	void XTEA_encrypt();
	void XTEA_decrypt();

	int m_MsgSize;
	int m_ReadPos;

	uint8_t m_MsgBuf[NETWORKMESSAGE_MAXSIZE];
	
	bool m_encryptionEnabled;
	bool m_keyset;
	uint32_t m_key[4];
	
	RSA* m_RSA;
};


#endif // #ifndef __NETWORK_MESSAGE_H__
