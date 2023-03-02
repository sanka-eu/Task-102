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

    //длинна равна нулю
    uint32_t length = 0;

    //создание объекта дл€ считывани€ данных из плоского массива data
    google::protobuf::io::CodedInputStream cis(reinterpret_cast<const uint8_t *>(data), size);
    
    //чтение длинны из сообщени€ Variant32
    cis.ReadVarint32(&length);

    //”становление лимита чтени€, чтобы не прочитать больше чем длинна сообщени€
    auto limit = cis.PushLimit(length);
    
    //currentposition возвращает текущую позицию относительно начала входного потока
    if (length + cis.CurrentPosition() > size) {
        return nullptr;
    }
    
    //подсчет количества использованных байт дл€ чтени€
    if (bytesConsumed) {
        *bytesConsumed = length + cis.CurrentPosition();
    }
    
    //создание экземпл€ра сообщени€
    shared_ptr<Message> message = std::make_shared<Message>();

    //заполнение сообщени€ буфером из данного потока и проверка использован ли весь ввод(ConsumedEntireMessage) а также ParseFromCodedStream вернет False если ошибка чтени€
    if(message->ParseFromCodedStream(&cis) && cis.ConsumedEntireMessage()) {
        //освобождение лимита чтени€, так как сообщение прочитано
        cis.PopLimit(limit);
        return message;
    }
    return nullptr;
}

#endif
