library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;

entity cleardisplay_init_data is
port (         
   i_idx   : in  unsigned(15 downto 0);
   --
   o_count : out unsigned(7 downto 0);
   o_data  : out std_logic_vector(7 downto 0)
);
end entity;

architecture cleardisplay_init_data_arch of cleardisplay_init_data is

   constant Size : integer := 1 + 7*10;
   type t_Data is array (0 to Size-1) of std_logic_vector(7 downto 0);
   constant s_lookup : t_Data := (
      X"12",
 
      X"18", X"00", X"02", X"00", X"00", X"80", X"40", X"01", X"00", X"00", -- clear screen 1
      X"18", X"10", X"02", X"00", X"00", X"80", X"40", X"00", X"01", X"00", -- clear screen 2
      X"18", X"20", X"02", X"00", X"00", X"80", X"40", X"00", X"00", X"01", -- clear screen 3
      X"18", X"30", X"02", X"00", X"00", X"80", X"40", X"01", X"01", X"00", -- clear screen 4
      X"18", X"40", X"02", X"00", X"00", X"80", X"40", X"01", X"00", X"01", -- clear screen 5
      X"18", X"50", X"02", X"00", X"00", X"80", X"40", X"00", X"01", X"01", -- clear screen 6
      X"18", X"60", X"02", X"00", X"00", X"80", X"40", X"01", X"01", X"01"  -- clear screen 7
   );

begin
   
   o_data  <= s_lookup(to_integer(i_idx));
   o_count <= to_unsigned(Size, o_count'length);

end architecture;
