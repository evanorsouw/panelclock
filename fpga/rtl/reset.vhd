library IEEE;
use IEEE.STD_LOGIC_1164.all;
use IEEE.NUMERIC_STD.all;

entity reset_controller is
port (    
      i_clk              : in std_logic;
      i_external_reset_n : in std_logic;
      --
      o_reset_n          : out std_logic
   );
end entity;

architecture reset_controller_arch of reset_controller is   
begin
   -- you cannot initialize variables or signals with explicit values, they all start at 0
   -- we count upwards to some value and while doing so we keep reset asserted.
   p_reset : process(i_external_reset_n, i_clk)
   variable v_reset_countup    : unsigned(29 downto 0);
   variable v_reset_n          : std_logic := '0';
   begin
      if i_external_reset_n = '0' then
         v_reset_n := '0';
         v_reset_countup := to_unsigned(0,v_reset_countup'length);
      elsif rising_edge(i_clk) then
        if v_reset_countup = 60000 then -- 1ms reset
            v_reset_n := '1';
        else
            v_reset_n := '0';
            v_reset_countup := v_reset_countup + 1;
        end if;
      end if;
      o_reset_n <= i_external_reset_n and v_reset_n;
   end process;

end architecture reset_controller_arch;