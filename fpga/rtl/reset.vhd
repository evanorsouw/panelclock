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

   -- you cannot initialize variables or signals with explicit values, they all start at 0
   -- we count upwards to some value and while doing so we keep reset asserted.
   signal s_reset_countup    : unsigned(16 downto 0);
   signal s_reset_n          : std_logic := '0';
   
begin
   p_reset : process(i_external_reset_n, i_clk)
   begin
      if i_external_reset_n = '0' then
         s_reset_n <= '0';
         s_reset_countup <= to_unsigned(0,s_reset_countup'length);
      elsif rising_edge(i_clk) then
        if s_reset_countup = 60000 then -- 1ms reset
            s_reset_n <= '1';
        else
            s_reset_n <= '0';
            s_reset_countup <= s_reset_countup + 1;
        end if;
      end if;
      o_reset_n <= i_external_reset_n and s_reset_n;
   end process;

end architecture reset_controller_arch;