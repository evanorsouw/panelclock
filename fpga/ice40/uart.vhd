library IEEE;
use IEEE.STD_LOGIC_1164.all;
use IEEE.NUMERIC_STD.all;

entity UART is
   port (   
      i_clkx8        : in std_logic;
      i_reset_n        : in std_logic;
      i_rx           : in std_logic;
      --
      o_datain       : out std_logic_vector (7 downto 0);
      o_datain_clk   : out std_logic
   );
end entity UART;

architecture UART_arch of UART is   

begin
   clock_proc: process (i_clkx8, i_reset_n)
   variable v_clk_count    : unsigned (3 downto 0);
   variable v_state        : unsigned (3 downto 0);
   variable v_shift_data   : std_logic_vector (7 downto 0);
   variable v_data_clk     : std_logic;
   begin
      if i_reset_n = '0' then
         v_state     := to_unsigned(0, v_state'length);
         v_data_clk  := '0';
            
      elsif rising_edge(i_clkx8) then
         v_data_clk  := '0';
         if (v_state = 0) then
            v_data_clk := '0';
            if (i_rx = '0') then
               v_state     := v_state + 1;
               v_clk_count := to_unsigned(12, v_clk_count'length);
            end if;
         
         else
            v_clk_count := v_clk_count - 1;            
            if (v_clk_count = 0) then
               v_clk_count := to_unsigned(8, v_clk_count'length);
               if (v_state <= 8) then
                  v_shift_data := i_rx & v_shift_data (7 downto 1);
                  v_state      := v_state + 1;               
                  
               elsif (i_rx = '1') then
                  v_state     := to_unsigned(0, v_state'length);
                  v_data_clk  := '1';
                                 
               else
                  v_state     := to_unsigned(0, v_state'length);
                  
               end if;      
            end if;
         end if;
      end if;      
      
      o_datain     <= v_shift_data;
      o_datain_clk <= v_data_clk;

   end process;   
end architecture UART_arch;
            
            
            
            