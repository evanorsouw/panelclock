library IEEE;
use IEEE.STD_LOGIC_1164.all;
use IEEE.NUMERIC_STD.all;

entity uart_tb is
end entity uart_tb;

architecture Behavioral of uart_tb is
  
   component UART
   port
   (
      i_clkx        : in std_logic;
      i_idle_ticks  : integer;
      i_reset_n     : in std_logic;
      i_rx          : in std_logic;
      --
      o_datain      : out std_logic_vector (7 downto 0);
      o_datain_clk  : out std_logic;
      --
      t_state           : out integer;
      t_duration        : out signed(31 downto 0);
      t_iread           : out integer;
      t_iwrite          : out integer;
      t_count           : out integer;
      t_minduration     : out signed(31 downto 0);
      t_active_duration : out signed(31 downto 0);
      t_bitcount        : out integer;
      t_assembled_byte  : out std_logic_vector(7 downto 0)
   );
   end component;

   constant CLOCK       : time := 13 us;
   constant BAUD        : time := 10.4 us;  -- 9600 baud
   constant IDLE_TIME   : time := BAUD * 30;

   -- module under test inputs
   signal tb_reset_n    : std_logic;
   signal tb_idle_ticks : integer;
   signal tb_clk        : std_logic;
   signal tb_uart_rx    : std_logic;
   
   -- module under test outputs;
   signal tb_state           : integer;
   signal tb_duration        : signed(31 downto 0);
   signal tb_iread           : integer;
   signal tb_iwrite          : integer;
   signal tb_count           : integer;
   signal tb_minduration     : signed(31 downto 0);
   signal tb_active_duration : signed(31 downto 0);
   signal tb_bitcount        : integer;
   signal tb_assembled_byte  : std_logic_vector(7 downto 0);
   signal tb_datain          : std_logic_vector(7 downto 0);
   signal tb_datain_clk      : std_logic;
  
   begin
   MUT: UART
   port map (
      i_clkx             => tb_clk,
      i_idle_ticks       => tb_idle_ticks,
      i_reset_n          => tb_reset_n,
      i_rx               => tb_uart_rx,
                             
      o_datain          => tb_datain,
      o_datain_clk      => tb_datain_clk,
      --
      t_state           => tb_state,
      t_duration        => tb_duration,
      t_iread           => tb_iread,
      t_iwrite          => tb_iwrite,
      t_count           => tb_count,
      t_minduration     => tb_minduration,
      t_active_duration => tb_active_duration,
      t_bitcount        => tb_bitcount,
      t_assembled_byte  => tb_assembled_byte
   );
     
   tb_clk <= '0' after CLOCK / 2 when tb_clk = '1' else
             '1' after CLOCK / 2;
             
   tb_idle_ticks <= IDLE_TIME / CLOCK;
  
   stimulate : process
   variable v_byte : std_logic_vector(7 downto 0);
   begin
      
-- _____     _             _     _             _ 
--      \_._/ \_._._._._._/ \_._/ \_._._._._._/ \
--       S 0 1 2 3 4 5 6 7 P S 0 1 2 3 4 5 6 7 P
--      |   | |           | |   | |           |
--         21 10         63 10 21 10         63
      
      for j in 1 to 10 loop

         tb_uart_rx <= '1';
         wait for IDLE_TIME * 3 / 2;
      
         v_byte := X"FF";
         tb_uart_rx <= '0';  -- start
         wait for BAUD;
         for k in 0 to 7 loop
            tb_uart_rx <= v_byte(k);            
            wait for BAUD;
         end loop;           
         tb_uart_rx <= '1';  -- stopbit
         wait for BAUD;

         wait for IDLE_TIME * 3 / 2;

         -- v_byte := X"02";
         -- tb_uart_rx <= '0';  -- start
         -- wait for BAUD;
         -- for k in 0 to 7 loop
            -- tb_uart_rx <= v_byte(k);            
            -- wait for BAUD;
         -- end loop;           
         -- tb_uart_rx <= '1';  -- stopbit
         -- wait for BAUD;

         -- v_byte := X"03";
         -- tb_uart_rx <= '0';  -- start
         -- wait for BAUD;
         -- for k in 0 to 7 loop
            -- tb_uart_rx <= v_byte(k);            
            -- wait for BAUD;
         -- end loop;           
         -- tb_uart_rx <= '1';  -- stopbit
         -- wait for BAUD;

         -- v_byte := X"04";
         -- tb_uart_rx <= '0';  -- start
         -- wait for BAUD;
         -- for k in 0 to 7 loop
            -- tb_uart_rx <= v_byte(k);            
            -- wait for BAUD;
         -- end loop;           
         -- tb_uart_rx <= '1';  -- stopbit
         -- wait for BAUD;
         
      end loop;
      
   end process;
  
   reset_gen : process
   begin
      for i in 1 to 5 loop
         if (i < 4) then
            tb_reset_n <= '0';
         else
            tb_reset_n <= '1';
         end if;
         wait until falling_edge(tb_clk);
       end loop;
       wait on tb_reset_n;
   end process reset_gen;
   
end architecture Behavioral;
  