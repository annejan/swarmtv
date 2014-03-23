#ifndef READABLESIZE_HPP
#define READABLESIZE_HPP

#include <cstring>

class readableSize
{
public:
  readableSize();
  readableSize(std::string &size);
  readableSize(size_t size);

  void setSize(std::string &size);
  void setSize(size_t size);

  void getSize(std::string &size);
  void getSize(size_t &size);
private:
  // Conversion functions
  void sizetohuman(size_t size, std::string &out);
  int humantosize(std::string &buf, size_t &size);

  // Store the size in bytes
  size_t mysize;
};

#endif // READABLESIZE_HPP
