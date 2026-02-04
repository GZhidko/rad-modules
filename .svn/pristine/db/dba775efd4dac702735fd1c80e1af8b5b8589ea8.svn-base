#ifndef MRADCLIENT_OUTPUT_BUFFER_H
#define MRADCLIENT_OUTPUT_BUFFER_H

class OutputBuffer {
private:
    size_t buffer_size;
    char * buffer;
    size_t buffer_len;
public:
    OutputBuffer();
    ~OutputBuffer();

    OutputBuffer(const OutputBuffer &);             // IMPLEMENT IT!!
    OutputBuffer & operator=(const OutputBuffer &); // IMPLEMENT IT!!

    //
    // интерфейс для диспетчера
    //
    size_t data_len() const;
    void * data_ptr() const;
    void data_cat(size_t len);

    //
    // интерфейс для поставщиков информации
    //
    void accept_data(char const * const data, size_t len);
};

#endif // MRADCLIENT_OUTPUT_BUFFER_H
