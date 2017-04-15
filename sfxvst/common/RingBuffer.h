//
//  The MIT License(MIT)
//
//  Copyright(c) 2016 Nocorupe
//
#pragma once

#include <array>
#include <cstring>

static const int MAX_SAMPLEFRAMES = 4096 * 2;

template<int BufferSize>
struct RingBuffer
{
	int _size;
	std::array<float,BufferSize> _data;
	int _point;
	int _forward;

	RingBuffer() :
		_size(BufferSize),
		_data(),
		_point(0),
		_forward(0)
	{
	}

	void init() {
		std::memset(_data.data(), 0, sizeof(float) * BufferSize);
	}

	inline float& get(int index) {
		if (_point + index < 0)
			return _data[_size + ((_point + index) % _size)];
		else
			return _data[(_point + index) % _size];
	}
	inline float& getForward(int index) {
		if (_forward + index < 0)
			return _data[_size + ((_forward + index) % _size)];
		else
			return _data[(_forward + index) % _size];
	}

	inline float& operator [](int index) {
		if (_point + index < 0)
			return _data[_size + ((_point + index) % _size)];
		else
			return _data[(_point + index) % _size];
	}

	void update(float* input, size_t data_size) {
		//for( size_t i = 0; i< data_size; i++ ) {

		//_data[ (_forward + i)% _size ] = input[i];
		//}
		int back_size = std::min<int>((int)data_size, _size - _forward);
		int front_size = std::max<int>(((_forward + (int)data_size) - _size) % (_size), 0);
		memcpy(_data.data() + _forward, input, sizeof(float) * back_size);
		memcpy(_data.data(), input + back_size, sizeof(float) * front_size);
		_point = _forward;
		_forward = (_forward + data_size) % _size;
	}

	
};

