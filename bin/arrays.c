//@ #include "arrays.gh"

/*@

lemma void array_split<t>(void *a, int offset)
    requires [?f]array<t>(a, ?n, ?size, ?q, ?as) &*& 0 <= offset &*& offset <= n;
    ensures [f]array<t>(a, offset, size, q, take(offset, as)) &*& [f]array<t>(a + (offset * size), n - offset, size, q, drop(offset, as));
{
  if (offset == 0) {
    take_0<t>(as);
    close [f]array<t>(a, 0, size, q, nil);
  } else {
    open [f]array<t>(a, n, size, q, as);
    array_split(a + size, offset - 1);
    close [f]array<t>(a, offset, size, q, take(offset, as));
  }
}

lemma void array_merge<t>(void *a)
    requires [?f]array<t>(a, ?M, ?size, ?q, ?as) &*& [f]array<t>(a + M * size, ?N, size, q, ?bs);
    ensures [f]array<t>(a, M + N, size, q, append(as, bs));
{
    open array<t>(a, M, size, q, as);
    if (M != 0) {
        array_merge((char*) a + size);
        close [f]array<t>(a, M + N, size, q, append(as, bs));
    }
}

lemma void array_unseparate_same<t>(void *a, list<t> xs)
    requires array<t>(a, ?M, ?size, ?q, take(M, xs)) &*& q(a + M * size, head(drop(M, xs))) &*& array<t>(a + (M *size) + size, ?N, size, q, tail(drop(M, xs))) &*& length(xs) == M + N + 1;
    ensures array<t>(a, M + N + 1, size, q, xs) &*& head(drop(M, xs)) == nth(M, xs);
{
    open array<t>(a, M, size, q, take(M, xs));
    switch (drop(M, xs)) { case nil: case cons(x0, xs0): }
    if (M != 0) {
        switch (xs) {
            case nil:
            case cons(h, t):
                array_unseparate_same(a + size, t);
                close array<t>(a, M + N + 1, size, q, xs);
        }
    }
}

lemma void array_unseparate<t>(void *a, int i, list<t> xs)
    requires array<t>(a, i, ?size, ?q, take(i, xs)) &*& q(a + i * size, ?y) &*& array<t>(a + (i + 1) * size, length(xs) - i - 1, size, q, tail(drop(i, xs)));
    ensures array<t>(a, length(xs), size, q, update(i, y, xs));
{
    open array<t>(a, i, size, q, take(i, xs));
    if (i == 0) {
        switch (xs) { case nil: case cons(x, xs0): }
    } else {
        switch (xs) { case nil: case cons(x, xs0): }
        array_unseparate(a + size, i - 1, tail(xs));
    }
    close array<t>(a, length(xs), size, q, update(i, y, xs));
}

lemma_auto void ints_inv()
    requires ints(?a, ?n, ?elems);
    ensures ints(a, n, elems) &*& 0 <= n &*& length(elems) == n;
{
    open ints(a, n, elems);
    close ints(a, n, elems);
}

lemma void ints_split(int *array, int offset)
    requires ints(array, ?N, ?as) &*& 0 <= offset &*& offset <= N;
    ensures ints(array, offset, take(offset, as)) &*& ints(array + offset, N - offset, drop(offset, as));
{
  open ints(array, N, as);
  array_split(array, offset);
  close ints(array, offset, take(offset, as));
  close ints(array + offset, N - offset, drop(offset, as));
}

lemma void ints_merge(int *a)
    requires ints(a, ?M, ?as) &*& ints(a + M, ?N, ?bs);
    ensures ints(a, M + N, append(as, bs));
{
    open ints(a, M, as);
    open ints(a + M, N, bs);
    array_merge(a);
    close ints(a, M + N, append(as, bs));
}

lemma void ints_unseparate_same(int *array, list<int> xs)
    requires ints(array, ?M, take(M, xs)) &*& integer(array + M, head(drop(M, xs))) &*& ints(array + M + 1, ?N, tail(drop(M, xs))) &*& length(xs) == M + N + 1;
    ensures ints(array, M + N + 1, xs) &*& head(drop(M, xs)) == nth(M, xs);
{
    open ints(array, M, _);
    open ints(array + M + 1, N, _);
    array_unseparate_same(array, xs);
    close ints(array, M + N + 1, xs);
}

lemma void ints_unseparate(int *array, int i, list<int> xs)
    requires ints(array, i, take(i, xs)) &*& integer(array + i, ?y) &*& ints(array + i + 1, length(xs) - i - 1, tail(drop(i, xs)));
    ensures ints(array, length(xs), update(i, y, xs));
{
    open ints(array, _, _);
    open ints(array + i + 1, _, _);
    array_unseparate(array, i, xs);
    close ints(array, length(xs), update(i, y, xs));
}


// ---- Arrays to chars ---- //
typedef lemma void any_to_chars<T>(predicate(void *p; T) any_pred, int length)(void *p);
    requires [?f]any_pred(p, _);
    ensures [f]chars(p, length, ?cs);

lemma void array_to_chars<T>(void *ptr)
    requires
        [?f]array<T>(ptr, ?array_nb_items, ?array_item_size, ?array_item_pred, ?array_elems)
        &*& is_any_to_chars(?convertor, array_item_pred, array_item_size);
    ensures
        [f]chars(ptr, array_nb_items * array_item_size, ?chars_elems)
        &*& is_any_to_chars(convertor, array_item_pred, array_item_size);
{
    if (array_nb_items == 0){
        open array<T>(ptr, 0, array_item_size, array_item_pred, array_elems);
        assert array_elems == nil;
        close [f]chars(ptr, 0, nil);    
    }else{
        // get array_item_pred(ptr) &*& array(ptr+array_item_size, ...)
        open array<T>(ptr, array_nb_items, array_item_size, array_item_pred, array_elems);
        
        // get array_item_pred(ptr) &*& chars(ptr+array_item_size,...)
        array_to_chars(ptr+array_item_size);
        
        // get chars(ptr) &*& chars(ptr+array_item_size,...)
        convertor(ptr);
        
        chars_join(ptr);
    }
}

lemma void u_int_array_to_chars(void *ptr)
    requires [?f]array<unsigned int>(ptr, ?array_nb_items, sizeof(unsigned int), u_integer, ?array_elems);
    ensures
        [f]chars(ptr, array_nb_items * sizeof(unsigned int), ?chars_elems);

{
    produce_lemma_function_pointer_chunk(u_integer_to_chars)
        : any_to_chars<unsigned int>(u_integer, sizeof(unsigned int))(args)
    {
        call();
    }{
        array_to_chars(ptr);
    }
}

lemma void u_character_to_chars(void *p)
    requires [?f]u_character(p, _);
    ensures [f]chars(p, sizeof(unsigned char), ?cs);
{
    u_character_to_character(p);
    assert [f]character(p, ?u_char_content);
    close [f]chars(p+sizeof(char), 0, nil);
    close [f]chars(p, 1, cons(u_char_content, nil));
}

lemma void u_char_array_to_chars(void *ptr)
    requires [?f]array<unsigned char>(ptr, ?array_nb_items, sizeof(unsigned char), u_character, ?elems);
    ensures
        [f]chars(ptr, array_nb_items * sizeof(unsigned char), ?chars_elems);
{
    produce_lemma_function_pointer_chunk(u_character_to_chars)
        : any_to_chars<unsigned char>(u_character, sizeof(unsigned char))(args)
    {
        call();
    }{
        array_to_chars(ptr);
    }
}

lemma void char_array_to_chars(void *ptr)
    requires [?f]array<char>(ptr, ?array_nb_items, sizeof(char), character, ?elems);
    ensures [f]chars(ptr, array_nb_items, elems);
{
    open array<char>(ptr, array_nb_items, sizeof(char), character, elems);
    if (array_nb_items != 0){
        char_array_to_chars(ptr + sizeof(char));
        assert [f]chars(ptr + sizeof(char), ?n, ?tail);
        assert [f]character(ptr, ?head);
        close [f]chars(ptr, n + 1, cons(head, tail));
    }else{
        close [f]chars(ptr, 0, nil);
    }
}

lemma void int_array_to_chars(void *ptr)
    requires [?f]array<int>(ptr, ?array_nb_items, sizeof(int), integer, ?elems);
    ensures
        [f]chars(ptr, array_nb_items * sizeof(int), ?chars_elems);
{
    produce_lemma_function_pointer_chunk(integer_to_chars)
        : any_to_chars<int>(integer, sizeof(int))(args)
    {
        call();
    }{
        array_to_chars(ptr);
    }
}

lemma void pointer_array_to_chars(void *ptr)
    requires [?f]array<void *>(ptr, ?array_nb_items, sizeof(void *), pointer, ?elems);
    ensures
        [f]chars(ptr, array_nb_items * sizeof(void *), ?chars_elems);
{
    produce_lemma_function_pointer_chunk(pointer_to_chars)
        : any_to_chars<void *>(pointer, sizeof(void *))(args)
    {
        call();
    }{
        array_to_chars(ptr);
    }
}


// ---- chars to arrays ---- //

typedef lemma void chars_to_any<T>(predicate(void *p; T) any_pred, int length)(void *p);
    requires [?f]chars(p, length, ?cs);
    ensures [f]any_pred(p, _);

lemma void chars_to_array<T>(void *ptr, int array_nb_items)
    requires
        is_chars_to_any<T>(?convertor, ?array_item_pred, ?array_item_length)
        &*& [?f]chars(ptr, array_nb_items * array_item_length, ?orig_elems)
        &*& array_nb_items >= 0
        &*& (array_item_length == 1 ? true : array_item_length == 2 ? true : array_item_length == 4 ? true : array_item_length == 8)
        //&*& array_item_length > 0 // Doesn't work, but e.g. "==4" works.
    ;
ensures
    [f]array<T>(ptr, array_nb_items, array_item_length, array_item_pred, ?array_elems)
    &*& length(array_elems) == array_nb_items
    &*& is_chars_to_any(convertor, array_item_pred, array_item_length);
{
    // We can't use induction here:
    // - Induction on chars isn't recognised because I call chars_split instead of opening chars
    // - induction on chars' elements isn't recognised
    // - induction on n isn't recognised
    // so we use a while-loop.
    
    void *array_ptr = ptr + array_nb_items * array_item_length; // Used imperatively.
    int array_length = 0; // Used imperatively.
    
    // start with empty array.
    close [f]array<T>(array_ptr, 0, array_item_length, array_item_pred, nil);
    
    while (array_length < array_nb_items)
        invariant
            [f]chars(ptr, (array_nb_items - array_length) * array_item_length, ?chars_elems)
            &*& [f]array<T>(array_ptr, array_length, array_item_length, array_item_pred, ?array_elems)
            
            // needed for chars_split
                &*& length(array_elems) == array_length
            // end.
            
            &*& array_ptr == ptr + (array_nb_items - length(array_elems)) * array_item_length
            &*& is_chars_to_any(convertor, array_item_pred, array_item_length)
        ;
        decreases array_nb_items - length(array_elems); // the variant.
    {
        array_ptr = array_ptr - array_item_length;
        int split_offset = array_ptr - ptr;
        chars_split(ptr, split_offset);
        
        convertor(array_ptr);
        
        array_length = array_length + 1;
        close [f]array<T>(array_ptr, array_length, array_item_length, array_item_pred, _);
    }
    
    open [f]chars(ptr, _, _);
}

lemma void chars_to_u_int_array(void *ptr, int array_nb_items)
    requires
        [?f]chars(ptr, array_nb_items * sizeof(unsigned int), ?orig_elems)
        &*& array_nb_items >= 0;
    ensures
        [f]array<unsigned int>(ptr, array_nb_items, sizeof(unsigned int), u_integer, ?orig_array_elems)
        &*& length(orig_array_elems) * sizeof(unsigned int) == length(orig_elems)
        &*& length(orig_array_elems) == array_nb_items;
{
    produce_lemma_function_pointer_chunk(chars_to_u_integer)
        : chars_to_any<unsigned int>(u_integer, sizeof(unsigned int))(args)
    {
        call();
    }{
        chars_to_array(ptr, array_nb_items);
    }
}

lemma void chars_to_u_character(void *p)
    requires [?f]chars(p, sizeof(unsigned char), ?cs);
    ensures  [f]u_character(p, _); 
{
    open chars(p, _, cs);
    open chars(p + sizeof(char), _, _);
    character_to_u_character(p);
}

lemma void chars_to_u_char_array(void *ptr)
    requires
        [?f]chars(ptr, ?n, ?orig_elems);
    ensures
        [f]array<unsigned char>(ptr, n, sizeof(unsigned char), u_character, ?orig_array_elems);
{
    produce_lemma_function_pointer_chunk(chars_to_u_character)
        : chars_to_any<unsigned char>(u_character, sizeof(unsigned char))(args)
    {
        call();
    }{
        chars_to_array(ptr, length(orig_elems));
    }
}

lemma void chars_to_int_array(void *ptr, int array_nb_items)
    requires
        [?f]chars(ptr, array_nb_items * sizeof(int), ?orig_elems)
        &*& array_nb_items >= 0;
    ensures
        [f]array<int>(ptr, array_nb_items, sizeof(int), integer, ?orig_array_elems)
        &*& length(orig_array_elems) * sizeof(int) == length(orig_elems)
        &*& length(orig_array_elems) == array_nb_items;
{
    produce_lemma_function_pointer_chunk(chars_to_integer)
        : chars_to_any<int>(integer, sizeof(int))(args)
    {
        call();
    }{
        chars_to_array(ptr, array_nb_items);
    }
}

lemma void chars_to_pointer_array(void *ptr, int array_nb_items)
    requires
        [?f]chars(ptr, array_nb_items * sizeof(void *), ?orig_elems);
    ensures
        [f]array<void *>(ptr, array_nb_items, sizeof(void *), pointer, ?orig_array_elems)
        &*& length(orig_array_elems) * sizeof(void *) == length(orig_elems)
        &*& length(orig_array_elems) == array_nb_items;
{
    produce_lemma_function_pointer_chunk(chars_to_pointer)
        : chars_to_any<void *>(pointer, sizeof(void *))(args)
    {
        call();
    }{
        chars_to_array(ptr, array_nb_items);
    }
}

lemma void chars_to_char_array(void *ptr)
    requires
        [?f]chars(ptr, ?n, ?elems);
    ensures
        [f]array<char>(ptr, length(elems), sizeof(char), character, elems);
{
    open chars(ptr, n, elems);
    if (n != 0){
        chars_to_char_array(ptr + sizeof(char));
        assert [f]array<char>(ptr + sizeof(char), n - 1, sizeof(unsigned char), character, ?tail);
        assert [f]character(ptr, ?head);
        close [f]array<char>(ptr, n, sizeof(char), character, cons(head, tail));
    }else{
        close [f]array<char>(ptr, 0, sizeof(char), character, nil);
    }
}

@*/
