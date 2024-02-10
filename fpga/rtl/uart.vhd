library IEEE;
use IEEE.STD_LOGIC_1164.all;
use IEEE.NUMERIC_STD.all;

entity UART is
   port (
      i_clkx         : in std_logic;
      i_reset_n      : in std_logic;
      i_ticks        : integer;        -- number of ticks per bit
      i_rx           : in std_logic;
      --
      o_datain       : out std_logic_vector (7 downto 0);
      o_datain_clk   : out std_logic
   );
end entity UART;

architecture UART_arch of UART is
begin
   clock_proc: process (i_clkx, i_reset_n)
   variable v_clk_count    : unsigned (15 downto 0);
   variable v_state        : unsigned (3 downto 0);
   variable v_shift_data   : std_logic_vector (7 downto 0);
   begin
      if i_reset_n = '0' then
         v_state     := to_unsigned(0, v_state'length);
         o_datain    <= "00000000";

      elsif rising_edge(i_clkx) then
         o_datain_clk <= '0';
         if (v_state = 0) then
            if (i_rx = '0') then
               v_state := v_state + 1;
               v_clk_count := to_unsigned(i_ticks * 3 / 2, v_clk_count'length);
            end if;
         else
            v_clk_count := v_clk_count - 1;
            if (v_clk_count = 0) then
               v_clk_count := to_unsigned(i_ticks, v_clk_count'length);
               if (v_state <= 8) then
                  v_shift_data := i_rx & v_shift_data (7 downto 1);
                  v_state := v_state + 1;
               elsif (i_rx = '1') then
                  v_state := to_unsigned(0, v_state'length);
                  o_datain_clk <= '1';
                  o_datain <=  v_shift_data;
               else
                  v_state := to_unsigned(0, v_state'length);
               end if;
            end if;
         end if;
      end if;

   end process;
end architecture UART_arch;



