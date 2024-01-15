library IEEE;
use IEEE.STD_LOGIC_1164.all;
use IEEE.NUMERIC_STD.all;

entity ledpanel_controller_tb is
end entity ledpanel_controller_tb;

architecture Behavioral of ledpanel_controller_tb is
  
   component ledpanel_controller is
   port (    
      i_clk30M      : in std_logic;
      i_reset_n     : in std_logic;
      i_uart_rx     : in std_logic;
      --
      o_dsp_clk     : out std_logic;
      o_dsp_latch   : out std_logic;
      o_dsp_oe_n    : out std_logic;
      o_dsp_addr    : out std_logic_vector (4 downto 0);
      o_dsp_rgbs    : out std_logic_vector (11 downto 0);
      o_dsp_vbl     : out std_logic;
      -- sram pins
      o_sram_oe_n   : out std_logic;
      o_sram_wr_n   : out std_logic;
      o_sram_cs_n   : out std_logic;
      o_sram_addr   : out std_logic_vector(13 downto 0);
      io_sram_data  : inout std_logic_vector(11 downto 0)
   );
   end component ledpanel_controller;

   constant RAM_SPEED   : time := 10 ns;
   constant CLOCK       : integer := 30000000;
   constant CLOCK_CYCLE : time := 1000 ms / CLOCK;
   constant BAUD        : time := 1000 ms / 115200;

   -- module under test inputs
   signal tb_clk        : std_logic;
   signal tb_reset_n    : std_logic;
   signal tb_uart_rx    : std_logic;
   
   -- module under test outputs;
   signal tb_dsp_clk    : std_logic;
   signal tb_dsp_latch  : std_logic;
   signal tb_dsp_oe_n   : std_logic;
   signal tb_dsp_addr   : std_logic_vector (4 downto 0);
   signal tb_dsp_rgbs   : std_logic_vector (11 downto 0);
   signal tb_dsp_vbl    : std_logic;
   signal tb_sram_oe_n  : std_logic;
   signal tb_sram_wr_n  : std_logic;
   signal tb_sram_cs_n  : std_logic;
   signal tb_sram_addr  : std_logic_vector(13 downto 0);
   signal tb_sram_data  : std_logic_vector(11 downto 0);
  
  type t_RAM is array (0 to 16383) of std_logic_vector(11 downto 0);
  signal ram : t_RAM := (others => (others => '1'));
  
   begin
   MUT: ledpanel_controller
   port map (
      i_clk30M              => tb_clk,
      i_reset_n             => tb_reset_n,
      i_uart_rx             => tb_uart_rx,
                             
      o_dsp_clk             => tb_dsp_clk,
      o_dsp_latch           => tb_dsp_latch,
      o_dsp_oe_n            => tb_dsp_oe_n,
      o_dsp_addr            => tb_dsp_addr,
      o_dsp_rgbs            => tb_dsp_rgbs,
      o_dsp_vbl             => tb_dsp_vbl,
                       
      o_sram_oe_n           => tb_sram_oe_n,
      o_sram_wr_n           => tb_sram_wr_n,
      o_sram_cs_n           => tb_sram_cs_n,
      o_sram_addr           => tb_sram_addr,
      io_sram_data          => tb_sram_data
   );
     
   tb_clk <= '0' after CLOCK_CYCLE/2 when tb_clk = '1' else
             '1' after CLOCK_CYCLE/2;

   p_ram : process(tb_sram_cs_n,tb_sram_wr_n, tb_sram_oe_n,tb_sram_data,tb_sram_addr)
   begin
      if tb_sram_cs_n = '1' then
         tb_sram_data <= (others => 'Z');
      elsif tb_sram_wr_n = '0' then 
         -- writing
         tb_sram_data <= (others => 'Z');
         ram(to_integer(unsigned(tb_sram_addr))) <= tb_sram_data;
      elsif tb_sram_oe_n = '1' then
         tb_sram_data <= (others => 'Z');
      else
         -- reading
         tb_sram_data <= ram(to_integer(unsigned(tb_sram_addr))) after RAM_SPEED;
      end if;
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

   uart: process
   variable v_byte : std_logic_vector(7 downto 0);
   begin
      wait on tb_reset_n;
      tb_uart_rx <= '1';
      
      wait for 10 ms;
      
      v_byte := X"02";
      tb_uart_rx <= '0';
      wait for BAUD;
      for k in 0 to 7 loop
         tb_uart_rx <= v_byte(k);            
         wait for BAUD;
      end loop;           
      tb_uart_rx <= '1';
      wait for BAUD;

      v_byte := X"02";
      tb_uart_rx <= '0';
      wait for BAUD;
      for k in 0 to 7 loop
         tb_uart_rx <= v_byte(k);            
         wait for BAUD;
      end loop;           
      tb_uart_rx <= '1';
      wait for BAUD;

      v_byte := X"02";
      tb_uart_rx <= '0';
      wait for BAUD;
      for k in 0 to 7 loop
         tb_uart_rx <= v_byte(k);            
         wait for BAUD;
      end loop;           
      tb_uart_rx <= '1';
      wait for BAUD;
      
      v_byte := X"08";
      tb_uart_rx <= '0';
      wait for BAUD;
      for k in 0 to 7 loop
         tb_uart_rx <= v_byte(k);            
         wait for BAUD;
      end loop;           
      tb_uart_rx <= '1';
      wait for BAUD;

      v_byte := X"08";
      tb_uart_rx <= '0';
      wait for BAUD;
      for k in 0 to 7 loop
         tb_uart_rx <= v_byte(k);            
         wait for BAUD;
      end loop;           
      tb_uart_rx <= '1';
      wait for BAUD;

      
      v_byte := X"FF";
      tb_uart_rx <= '0';
      wait for BAUD;
      for k in 0 to 7 loop
         tb_uart_rx <= v_byte(k);            
         wait for BAUD;
      end loop;           
      tb_uart_rx <= '1';
      wait for BAUD;
      
      v_byte := X"00";
      tb_uart_rx <= '0';
      wait for BAUD;
      for k in 0 to 7 loop
         tb_uart_rx <= v_byte(k);            
         wait for BAUD;
      end loop;           
      tb_uart_rx <= '1';
      wait for BAUD;

      v_byte := X"00";
      tb_uart_rx <= '0';
      wait for BAUD;
      for k in 0 to 7 loop
         tb_uart_rx <= v_byte(k);            
         wait for BAUD;
      end loop;           
      tb_uart_rx <= '1';
      wait for BAUD;
      
      
   end process;
  
end architecture Behavioral;
  