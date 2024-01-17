library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;

entity whitemagic_init_screen is
port (         
   i_idx   : in  unsigned(15 downto 0);
   --
   o_count : out unsigned(7 downto 0);
   o_data  : out std_logic_vector(7 downto 0)
);
end entity;

architecture whitemagic_init_screen_arch of whitemagic_init_screen is

   constant Size : integer := 17*8;
   type t_Data is array (0 to Size-1) of std_logic_vector(7 downto 0);
   constant s_lookup : t_Data := (
      X"02", X"00", X"00", X"40", X"40", X"1f", X"1f", X"1f",  -- white background
      
      X"02", X"06", X"27", X"13", X"01", X"00", X"00", X"00",  -- black top line
      X"02", X"05", X"28", X"15", X"13", X"00", X"00", X"00",  -- black centre
      X"02", X"06", X"3B", X"13", X"01", X"00", X"00", X"00",  -- black bottom line
      
      X"02", X"01", X"2D", X"04", X"04", X"00", X"00", X"00",  -- top connector
      X"02", X"02", X"2E", X"06", X"02", X"1f", X"1f", X"1f",  -- top connector infill
      X"02", X"01", X"34", X"04", X"04", X"00", X"00", X"00",  -- bottom connector      
      X"02", X"02", X"35", X"06", X"02", X"1f", X"1f", X"1f",  -- bottom connector infill

      X"02", X"0A", X"32", X"04", X"04", X"FF", X"FF", X"FF",  -- white square 1
      X"02", X"12", X"32", X"04", X"04", X"FF", X"FF", X"FF",  -- white square 2
      X"02", X"0E", X"2D", X"04", X"04", X"FF", X"FF", X"FF",  -- white square 3
      X"02", X"14", X"28", X"04", X"04", X"FF", X"FF", X"FF",  -- white square 4
      X"02", X"0D", X"25", X"04", X"02", X"00", X"00", X"00",  -- white square 5 top
      X"02", X"0D", X"27", X"04", X"02", X"FF", X"FF", X"FF",  -- white square 5 bottom
      
      X"02", X"17", X"1C", X"04", X"04", X"00", X"A5", X"E0",  -- blue square
      X"02", X"12", X"20", X"04", X"04", X"FF", X"00", X"00",  -- red square
      X"02", X"18", X"22", X"04", X"04", X"83", X"C3", X"0E"   -- green square     
   );

begin
   
   o_data  <= s_lookup(to_integer(i_idx));
   o_count <= to_unsigned(Size, o_count'length);

end architecture;
