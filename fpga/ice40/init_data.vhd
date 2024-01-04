library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;

entity whitemagic_init_screen is
   port (         
      i_idx   : in  unsigned(7 downto 0);
      o_count : out unsigned(7 downto 0);
      o_data  : out std_logic_vector(7 downto 0)
   );
end entity;

architecture whitemagic_init_screen_arch of whitemagic_init_screen is

   constant Size : integer := 14*8;
   type t_Data is array (0 to Size-1) of std_logic_vector(7 downto 0);
   constant s_lookup : t_Data := (
      X"02", X"00", X"00", X"40", X"40", X"1F", X"1F", X"1F",  -- white background
      X"02", X"05", X"0A", X"15", X"01", X"00", X"00", X"00",  -- black top line
      X"02", X"04", X"0B", X"17", X"15", X"00", X"00", X"00",  -- black section
      X"02", X"05", X"20", X"15", X"01", X"00", X"00", X"00",  -- black bottom line
      X"02", X"01", X"12", X"07", X"04", X"00", X"00", X"00",    
      X"02", X"02", X"13", X"05", X"02", X"FF", X"FF", X"FF",    
      X"02", X"01", X"18", X"07", X"04", X"00", X"00", X"00",    
      X"02", X"02", X"19", X"05", X"02", X"FF", X"FF", X"FF",    

      X"02", X"14", X"0C", X"04", X"04", X"FF", X"FF", X"FF",  -- w2hite square 1
      X"02", X"0B", X"15", X"04", X"04", X"FF", X"FF", X"FF",  -- w2hite square 1
      X"02", X"12", X"16", X"04", X"04", X"FF", X"FF", X"FF",  -- w2hite square 1
      
      X"02", X"18", X"01", X"04", X"04", X"00", X"A5", X"E0",  -- blue square
      X"02", X"11", X"01", X"04", X"04", X"FF", X"00", X"00",  -- red square
      X"02", X"16", X"07", X"04", X"04", X"83", X"C3", X"0E"   -- green square
   );

begin
   
   o_data  <= s_lookup(to_integer(i_idx));
   o_count <= to_unsigned(Size, o_count'length);

end architecture;
