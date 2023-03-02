#ifndef DELIMITEDMESSAGESSTREAMPARSER_HPP
#define DELIMITEDMESSAGESSTREAMPARSER_HPP

#include "helpers.hpp"

#include <list>
#include <vector>
#include <string>
#include <boost/make_shared.hpp>

using std::list;
using std::vector;
using std::string;
using std::shared_ptr;

template<typename Message>
class DelimitedMessagesStreamParser {
public:
    typedef shared_ptr<const Message> PointerToConstValue;

    //метод parse принимает строку data
    list<PointerToConstValue> parse(const string & data) {
        
        //заполнение буфера принятой строкой
        for (const char byte : data) {
            m_buffer.push_back(byte);
        }

        //объявление списка сообщений
        list<typename DelimitedMessagesStreamParser<Message>::PointerToConstValue> messages;
        
        //переменная для занесения количества использованных байт
        size_t bytesConsumed = 0;

        //пока размер буфера не равен нулю ходим по циклу
        while (m_buffer.size()) {
            //извлечение одного сообщения
            shared_ptr<Message> message = parseDelimited<Message>(m_buffer.data(), m_buffer.size(), &bytesConsumed);
            
            //если вернулся не nullptr добавляем в список сообщений
            if (message) {
                messages.push_back(message);
            }
            
            //если не понадобилось байт для извлечения сообщений, завершаем чтение
            if (bytesConsumed == 0) {
                break;
            }
            
            //очистка буфера от прочитанного сообщения
            vector<char>(m_buffer.begin() + bytesConsumed, m_buffer.end()).swap(m_buffer);
            
            //зануление количества использованных байт
            bytesConsumed = 0;
        }

        return messages;
    }

private:
    vector<char> m_buffer;
};

#endif
