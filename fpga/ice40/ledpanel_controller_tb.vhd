library IEEE;
use IEEE.STD_LOGIC_1164.all;
use IEEE.NUMERIC_STD.all;

entity ledpanel_controller_tb is
end entity ledpanel_controller_tb;

architecture Behavioral of ledpanel_controller_tb is
  
   component ledpanel_controller is
   port (    
      i_clk180M     : in std_logic;
      i_reset_n     : in std_logic;
      i_uart_rx     : in std_logic;
      --
      o_dsp_clk     : out std_logic;
      o_dsp_latch   : out std_logic;
      o_dsp_oe      : out std_logic;
      o_dsp_addr    : out std_logic_vector (4 downto 0);
      o_dsp_rgbs    : out std_logic_vector (11 downto 0);
      o_dsp_vbl     : out std_logic;
      -- sram pins
      o_sram_oe     : out std_logic;
      o_sram_wr     : out std_logic;
      o_sram_cs     : out std_logic;
      o_sram_addr   : out std_logic_vector(14 downto 0);
      io_sram_data  : inout std_logic_vector(11 downto 0)
   );
   end component ledpanel_controller;

   constant HALF_PERIOD : time := 2.78 ns;  -- 180MHz = 5.56ns

   -- module under test inputs
   signal tb_clk        : std_logic;
   signal tb_reset      : std_logic;
   signal tb_uart_rx    : std_logic;
   
   -- module under test outputs;
   signal tb_dsp_clk    : std_logic;
   signal tb_dsp_latch  : std_logic;
   signal tb_dsp_oe     : std_logic;
   signal tb_dsp_addr   : std_logic_vector (4 downto 0);
   signal tb_dsp_rgbs   : std_logic_vector (11 downto 0);
   signal tb_dsp_vbl    : std_logic;
   signal tb_sram_oe    : std_logic;
   signal tb_sram_wr    : std_logic;
   signal tb_sram_cs    : std_logic;
   signal tb_sram_addr  : std_logic_vector(14 downto 0);
   signal tb_sram_data_instant  : std_logic_vector(11 downto 0);
   signal tb_sram_data  : std_logic_vector(11 downto 0);
  
  type t_RAM is array (0 to 32767) of std_logic_vector(11 downto 0);
  signal ram : t_RAM := (others => (others => '1'));
  
   begin
   MUT: ledpanel_controller
   port map (
      i_clk180M             => tb_clk,
      i_reset_n             => tb_reset,
      i_uart_rx             => tb_uart_rx,
                             
      o_dsp_clk             => tb_dsp_clk,
      o_dsp_latch           => tb_dsp_latch,
      o_dsp_oe              => tb_dsp_oe,
      o_dsp_addr            => tb_dsp_addr,
      o_dsp_rgbs            => tb_dsp_rgbs,
      o_dsp_vbl             => tb_dsp_vbl,
                       
      o_sram_oe             => tb_sram_oe,
      o_sram_wr             => tb_sram_wr,
      o_sram_cs             => tb_sram_cs,
      o_sram_addr           => tb_sram_addr,
      io_sram_data          => tb_sram_data
   );
     
   tb_clk <= '0' after HALF_PERIOD when tb_clk = '1' else
             '1' after HALF_PERIOD;

   p_ram : process(tb_clk)
   begin
      if tb_clk'event then
         if tb_sram_cs = '1' then
            tb_sram_data_instant <= (others => 'Z');
            tb_sram_data <= (others => 'Z');
         elsif tb_sram_wr = '0' then 
            -- writing
            tb_sram_data <= (others => 'Z');
            tb_sram_data_instant <= (others => 'Z');
            ram(to_integer(unsigned(tb_sram_addr))) <= tb_sram_data;
         elsif tb_sram_oe = '1' then
            tb_sram_data_instant <= (others => 'Z');
            tb_sram_data <= (others => 'Z');
         else
            -- reading
            tb_sram_data_instant <= ram(to_integer(unsigned(tb_sram_addr)));
            tb_sram_data <= ram(to_integer(unsigned(tb_sram_addr)));
         end if;
      end if;
   end process;
   
   reset_gen : process
   begin
      for i in 1 to 5 loop
         if (i < 4) then
            tb_reset <= '0';
         else
            tb_reset <= '1';
         end if;
         wait until falling_edge(tb_clk);
       end loop;
       wait on tb_reset;
   end process reset_gen;
   
end architecture Behavioral;
  