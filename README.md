Built with LLVM 14.0.0

CharHead is an esoteric language that is defined by a small set of symbols.

### Symbols 

  ##### `#`: increment byte at pointer

  ##### `$`: decrement byte at pointer

  ##### `!`: increment pointer

  ##### `@`: decrement pointer

  ##### `;`: print character

  ##### `_`: input char and store at pointer

  ##### `(`: start loop, execute code until byte at pointer is 0

  ##### `)`: end loop, jump back to matching [
  
  ##### `[`: pushes current byte at pointer to operation stack

  ##### `]`: pops operation stack and assigns to pointer 
  
  ##### `.`: peek operation stack