# Project 3 Design Log
**Author:** Christian Barron-Anaya · **P2 code used:** My own
**Tools used:** CLion debugger (for debugging code)

## 1. Hash table
The hash table uses chaining with `std::vector` as the chain.
Chaining was chosen because the degradation in speed is slower than open 
addressing as the table fills up, due to the generally smaller chain sizes 
compared to the capacity of the table. Chaining also makes operations like 
`find()` and `add()` easier to implement, as they only have to search in 
a specific bucket's vector. 

The string hash function chosen was FNV-1a, which gives a 64-bit hash. This 
hash was chosen for its simplicity and uniformity in mapping any length of
string to a random 64-bit integer. 

The load-factor threshold chosen was 0.75. A lower factor was chosen to prevent
the table from being too full while not being too low and causing too many 
unnecessary resizes.

The table resizes when the load-factor crosses the threshold above. The table's 
`numBuckets` is doubled, and all of the previous `Tool` unique pointers are 
reinserted. Doubling was chosen, as keeping the table's `numBuckets` a power of 2
allows the use of `hash & (bucket_count()-1)` to cheaply compute the index.

In adding 10000 tools, named "tool_1", "tool_2", etc., the FNV-1a hash achieved 
a final `numBuckets` of 16384, `load_factor` of 0.610352, and `max_probe_length` 
of 6. The "hash" method of `s[0] + s.size()` had a max chain of 9000, as the 
hash value for `tool_1000` through `tool_9999` is the same. This is because
the starting character and length are the same, which are the only things 
considered with this "hash".

The `add()` operation achieves an amortized O(1) due to it not needing to resize
on every operation. Inserting an element that would cross the load-factor threshold
would cost O(N) time, where N is the size of the table, as every element would need 
to be reinserted to the table. However, since the table size has doubled, each 
subsequent add happens in O(1) time. If the initial size of the table was *n*, then
after the resize, another resize won't happen until the table is of size *2n*. 
Since the sum of the costs of the cheap inserts over N elements is O(N), the
amortized cost is O(N)/N = O(1).

## 2. Pointer stability
To prevent tools from getting lost after a hash table resize, the hash table only 
stores `std::unique_ptr<Tool>` in the table. During a resize, these unique pointers,
and not the `Tool` itself, are moved from the old table into the new table.


## 3. Calculator trace
*Stacks are shown with the top of the stack as the first in the list*

**At the start:**

values: (empty), ops: (empty)

**Token: `2`**

`2` gets pushed onto `values`.

values: `2`, ops: (empty)

**Token: `*`**

`ops` stack is empty, `*` gets pushed onto `ops`.

values: `2`, ops: `*`

**Token: `(`**

`(` gets pushed onto `ops`.

values: `2`, ops: `( *`

**Token: `3`**

`3` gets pushed onto `values`.

values: `3 2`, ops: `( *`

**Token: `+`**

the top of the `ops` stack has `(`, `+` gets pushed onto `ops`.

values: `3 2`, ops: `+ ( *`

**Token: `4`**

`4` gets pushed onto `values`.

values: `4 3 2`, ops: `+ ( *`

**Token: `)`**

Since top of `ops` is not `(`, apply (pop two operands and operator). This gives `3 + 4` which is `7`. Push `7` onto `values`.

values: `7 2`, ops: `( *`

Top of `ops` is `(`, pop `(` from `ops`.

values: `7 2`, ops: `*`

**Token: `-`**

Precedence of `*` (2) is greater than or equal to precedence of `-` (1). 
Apply (pop two operands and operator). This gives `2 * 7`, which is `14`.
Push `14` onto `values`.
Push `-` onto `ops`.

values: `14`, ops: `-`

**Token: `5`**

`5` gets pushed onto `values`.

values: `5 14`, ops: `-`

Token stream ended, apply remaining operators to values.
This gives `14 - 5` which is `9`.
Push `9` onto `values`.

values: `9`, ops: (empty)

The final result is **9**.

## 4. Errors as values
Tools, `resolve()`, and `calc_evaluate()` all *return* errors, instead of just
throwing them. This is done because a certain amount of tool errors are expected.
Instead of the program throwing an error (or even outright crashing), an error is
returned when the tool cannot process the request (possibly due to malformed data,
or other things such as a divide by 0 with otherwise perfectly valid data) to signal
to the model to try something else. 

An error is reserved for the programmer to indicate something is wrong with the code,
not the data input into the tool. As long as it is implemented correctly, 
this split makes it easier to determine the source of the problem, knowing the tool
will handle just about any data input without crashing.

## 5. What I would design differently
I would do more research on the best parameters for a hash table with chaining (and 
potentially try out more values for the load factor), and also should have settled on
a collision strategy earlier to make developing the member functions and tests easier.