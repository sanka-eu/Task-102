#ifndef HELPERS_HPP
#define HELPERS_HPP

#include <boost/make_shared.hpp>
#include <google/protobuf/io/coded_stream.h>

using std::shared_ptr;

template<typename Message>
shared_ptr<Message> parseDelimited(const void * data, size_t size, size_t * bytesConsumed = 0) {
    if (data == nullptr) {
        return nullptr;
    }

    //������ ����� ����
    uint32_t length = 0;

    //�������� ������� ��� ���������� ������ �� �������� ������� data
    google::protobuf::io::CodedInputStream cis(reinterpret_cast<const uint8_t *>(data), size);
    
    //������ ������ �� ��������� Variant32
    cis.ReadVarint32(&length);

    //������������ ������ ������, ����� �� ��������� ������ ��� ������ ���������
    auto limit = cis.PushLimit(length);
    
    //currentposition ���������� ������� ������� ������������ ������ �������� ������
    if (length + cis.CurrentPosition() > size) {
        return nullptr;
    }
    
    //������� ���������� �������������� ���� ��� ������
    if (bytesConsumed) {
        *bytesConsumed = length + cis.CurrentPosition();
    }
    
    //�������� ���������� ���������
    shared_ptr<Message> message = std::make_shared<Message>();

    //���������� ��������� ������� �� ������� ������ � �������� ����������� �� ���� ����(ConsumedEntireMessage) � ����� ParseFromCodedStream ������ False ���� ������ ������
    if(message->ParseFromCodedStream(&cis) && cis.ConsumedEntireMessage()) {
        //������������ ������ ������, ��� ��� ��������� ���������
        cis.PopLimit(limit);
        return message;
    }
    return nullptr;
}

#endif
