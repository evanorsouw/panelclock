library IEEE;
use IEEE.STD_LOGIC_1164.all;
use IEEE.NUMERIC_STD.all;

entity UART is
   port (   
      clkx8         : in std_logic;
      reset         : in std_logic;
      rx            : in std_logic;
      --
      datain        : out std_logic_vector (7 downto 0);
      datain_clk   : out std_logic
   );
end entity UART;

architecture UART_arch of UART is   

begin
   clock_proc: process (clkx8, reset)
   variable clk_count    : unsigned (3 downto 0);
   variable state        : unsigned (3 downto 0);
   variable shift_data   : std_logic_vector (7 downto 0);
   variable data_clk     : std_logic;
   begin
      if reset = '1' then
         state     := to_unsigned(0, state'length);
         data_clk  := '0';
            
      elsif rising_edge(clkx8) then
         data_clk  := '0';
         if (state = 0) then
            data_clk := '0';
            if (rx = '0') then
               state     := state + 1;
               clk_count := to_unsigned(12, clk_count'length);
            end if;
         
         else
            clk_count := clk_count - 1;            
            if (clk_count = 0) then
               clk_count := to_unsigned(8, clk_count'length);
               if (state <= 8) then
                  shift_data := rx & shift_data (7 downto 1);
                  state      := state + 1;               
                  
               elsif (rx = '1') then
                  state     := to_unsigned(0, state'length);
                  data_clk  := '1';
                                 
               else
                  state     := to_unsigned(0, state'length);
                  
               end if;      
            end if;
         end if;
      end if;      
      
      datain     <= shift_data;
      datain_clk <= data_clk;

   end process;   
end architecture UART_arch;
            
            
            
            