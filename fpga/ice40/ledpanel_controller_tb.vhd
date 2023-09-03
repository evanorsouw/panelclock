library IEEE;
use IEEE.STD_LOGIC_1164.all;
use IEEE.NUMERIC_STD.all;

entity ledpanel_controller_tb is
end entity ledpanel_controller_tb;

architecture Behavioral of ledpanel_controller_tb is
  
   component ledpanel_controller is
   port (    
      clk180M     : in std_logic;
      reset       : in std_logic;
      uart_rx     : in std_logic;
      --
      dsp_clk     : out std_logic;
      dsp_latch   : out std_logic;
      dsp_oe      : out std_logic;
      dsp_addr    : out std_logic_vector (4 downto 0);
      dsp_rgbs    : out std_logic_vector (11 downto 0);
      dsp_vbl     : out std_logic;
      -- sram pins
      sram_oe     : out std_logic;
      sram_wr     : out std_logic;
      sram_cs     : out std_logic;
      sram_addr   : out std_logic_vector(14 downto 0);
      sram_data   : inout std_logic_vector(11 downto 0);
      -- test
      tst_fifo_ren    : out std_logic;
      tst_fifo_dataout: out std_logic_vector(7 downto 0);
      tst_fifo_wen    : out std_logic;
      tst_ram_wr_clk  : out std_logic;
      tst_ram_wr_data : out std_logic_vector(7 downto 0);
      tst_ram_wr_mask  : out std_logic_vector(11 downto 0);
      tst_color_update_done : out std_logic
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
   signal tb_sram_data  : std_logic_vector(11 downto 0);
   signal tb_sram_data_delayed  : std_logic_vector(11 downto 0);
   signal tb_fifo_ren   : std_logic;
   signal tb_fifo_dataout:std_logic_vector(7 downto 0);
   signal tb_fifo_wen   : std_logic;
   signal tb_ram_wr_clk : std_logic;
   signal tb_ram_wr_data:std_logic_vector(7 downto 0);
   signal tb_ram_wr_mask:std_logic_vector(11 downto 0);
   signal tb_color_update_done:std_logic;
   
   begin
   MUT: ledpanel_controller
   port map (
      clk180M    => tb_clk,
      reset      => tb_reset,
      uart_rx    => tb_uart_rx,
                    
      dsp_clk    => tb_dsp_clk,
      dsp_latch  => tb_dsp_latch,
      dsp_oe     => tb_dsp_oe,
      dsp_addr   => tb_dsp_addr,
      dsp_rgbs   => tb_dsp_rgbs,
      dsp_vbl    => tb_dsp_vbl,
      sram_oe    => tb_sram_oe,
      sram_wr    => tb_sram_wr,
      sram_cs    => tb_sram_cs,
      sram_addr  => tb_sram_addr,
      sram_data  => tb_sram_data_delayed,
      tst_fifo_ren   => tb_fifo_ren,
      tst_fifo_dataout   => tb_fifo_dataout,
      tst_fifo_wen   => tb_fifo_wen,
      tst_ram_wr_clk => tb_ram_wr_clk,
      tst_ram_wr_data=> tb_ram_wr_data,
      tst_ram_wr_mask=> tb_ram_wr_mask,
      tst_color_update_done => tb_color_update_done      
   );
     
   tb_clk <= '0' after HALF_PERIOD when tb_clk = '1' else
             '1' after HALF_PERIOD;

   tb_sram_data <= tb_sram_addr(11 downto 0) 
      when tb_sram_oe = '0' and tb_sram_wr = '1' and tb_sram_cs = '0' 
      else (others => 'Z');
   tb_sram_data_delayed <= transport tb_sram_data after 10 ns;
   
   reset_gen : process
   begin
      for i in 1 to 5 loop
         if (i < 4) then
            tb_reset <= '1';
         else
            tb_reset <= '0';
         end if;
         wait until falling_edge(tb_clk);
       end loop;
       wait on tb_reset;
   end process reset_gen;
   
end architecture Behavioral;
  