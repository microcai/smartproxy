
#pragma once

template <typename Iterator>
class iterator_pair {
public:
	iterator_pair ( Iterator first, Iterator last ) : f_ (first), l_ (last) {}
	Iterator begin () const { return f_; }
	Iterator end   () const { return l_; }

private:
	Iterator f_;
	Iterator l_;
};

template <typename Iterator>
iterator_pair<Iterator> make_iterator_pair ( Iterator f, Iterator l ) {
	return iterator_pair<Iterator> ( f, l );
}
