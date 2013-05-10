#ifndef  __SW_USB_TYPEDEF_H__
#define  __SW_USB_TYPEDEF_H__


typedef signed char           int8;
typedef signed short          int16;
typedef signed int            int32;
typedef unsigned char         uint8;
typedef unsigned short        uint16;
typedef unsigned int          uint32;
typedef unsigned int        __hdle;

#undef  x_set_bit
#define x_set_bit( value, bit )      		( (value) |=  ( 1U << (bit) ) )

#undef  x_clear_bit
#define x_clear_bit( value, bit )    		( (value) &= ~( 1U << (bit) ) )

#undef  x_reverse_bit
#define x_reverse_bit( value, bit )  		( (value) ^=  ( 1U << (bit) ) )

#undef  x_test_bit
#define x_test_bit( value, bit )     		( (value)  &  ( 1U << (bit) ) )

#undef  x_min
#define x_min( x, y )          				( (x) < (y) ? (x) : (y) )

#undef  x_max
#define x_max( x, y )          				( (x) > (y) ? (x) : (y) )

#undef  x_absolute
#define x_absolute(p)        				((p) > 0 ? (p) : -(p))

#endif   //__SW_USB_TYPEDEF_H__

