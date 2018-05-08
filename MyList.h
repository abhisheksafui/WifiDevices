#include <LinkedList.h>

template <typename T>
class MyList: public LinkedList<T> {


public: 

  void remove_if(std::function<bool(T)> func)
  {
    ListNode<T> *current = root;  
  }
  
}

