#include "pch.h"
#include "SendBuffer.h"

SendBuffer::SendBuffer(SendBufferChunkRef owner, BYTE* buffer, uint32 allocSize)
	: _owner(owner), _buffer(buffer), _allocSize(allocSize)
{
}

SendBuffer::~SendBuffer()
{
}

void SendBuffer::Close(uint32 writeSize)
{
	ASSERT_CRASH(writeSize <= _allocSize);
	_writeSize = writeSize;
	_owner->Close(writeSize);
}

/*----------------------
	SendBufferChunk
-----------------------*/

SendBufferChunk::SendBufferChunk()
{
}

SendBufferChunk::~SendBufferChunk()
{
}

void SendBufferChunk::Reset()
{
	_open = false;
	_usedSize = 0;
}

SendBufferRef SendBufferChunk::Open(uint32 allocSize)
{
	ASSERT_CRASH(allocSize <= SEND_BUFFER_CHUNK_SIZE);
	ASSERT_CRASH(_open == false);

	if (allocSize > FreeSize())
		return nullptr;

	_open = true;
	return ObjectPool<SendBuffer>::MakeShared(shared_from_this(), Buffer(), allocSize);
}

void SendBufferChunk::Close(uint32 writeSize)
{
	ASSERT_CRASH(_open == true);
	_open = false;
	_usedSize += writeSize;
}

/*----------------------
	SendBufferManager
-----------------------*/

SendBufferManager::SendBufferManager()
{
}

SendBufferManager::~SendBufferManager()
{
	Shutdown();
}

SendBufferRef SendBufferManager::Open(uint32 size)
{
	ASSERT_CRASH(_shuttingDown.load() == false);

	if (LSendBufferChunk == nullptr)
	{
		LSendBufferChunk = Pop(); // Write_Lock
		LSendBufferChunk->Reset();
	}

	ASSERT_CRASH(LSendBufferChunk->IsOpen() == false);

	// 다 썼으면 버리고 새거로 교체
	if (LSendBufferChunk->FreeSize() < size)
	{
		LSendBufferChunk = Pop();
		LSendBufferChunk->Reset();
	}

	//cout << "FREE : " << LSendBufferChunk->FreeSize() << endl;

	return LSendBufferChunk->Open(size);
}

void SendBufferManager::Shutdown()
{
	if (_shuttingDown.exchange(true))
		return;

	std::vector<SendBufferChunkRef> sendBufferChunks;
	{
		WRITE_LOCK;
		sendBufferChunks.reserve(_sendBufferChunks.size());
		for (SendBufferChunkRef& sendBufferChunk : _sendBufferChunks)
			sendBufferChunks.push_back(std::move(sendBufferChunk));

		_sendBufferChunks.clear();
	}

	// PushGlobal이 다시 manager로 반납하지 않고 실제 메모리를 해제한다.
	sendBufferChunks.clear();
}

SendBufferChunkRef SendBufferManager::Pop()
{
	{
		WRITE_LOCK;
		if (_sendBufferChunks.empty() == false)
		{
			SendBufferChunkRef sendBufferChunk = _sendBufferChunks.back();
			_sendBufferChunks.pop_back();
			return sendBufferChunk;
		}
	}

	return SendBufferChunkRef(xnew<SendBufferChunk>(), PushGlobal);
}

void SendBufferManager::Push(SendBufferChunkRef buffer)
{
	WRITE_LOCK;
	_sendBufferChunks.push_back(buffer);
}

void SendBufferManager::PushGlobal(SendBufferChunk* buffer)
{
	SendBufferManager* manager = GSendBufferManager;
	if (manager != nullptr && manager->_shuttingDown.load() == false)
	{
		manager->Push(SendBufferChunkRef(buffer, PushGlobal));
		return;
	}

	xdelete(buffer);
}
