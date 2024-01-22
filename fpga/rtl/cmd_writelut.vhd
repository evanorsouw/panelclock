library IEEE;
use IEEE.STD_LOGIC_1164.all;
use IEEE.NUMERIC_STD.all;

entity cmd_rgblut is
   port (   
      i_reset_n        : in std_logic;
      i_clk            : in std_logic;
      i_data           : in std_logic_vector(7 downto 0);
      i_data_rdy       : in std_logic; -- indication that new data is available
      --
      o_busy           : out std_logic; -- indication that command is in progress
      o_need_more_data : out std_logic; -- indication that more data is needed
      o_rgb            : out std_logic_vector(23 downto 0);
      o_write_clk      : out std_logic  -- on rising edge, address and rgb values are ready for writing
   );
end entity cmd_rgblut;

-- command fills the lookup RGB LUT with a single value.
-- expected byte sequence: 0x10,<rgb>,<red>,<green>,<blue>
architecture cmd_rgblut_arch of cmd_rgblut is   

   type T_STATE is ( CMD, ARG_IDX, ARG_RED, ARG_GRN, ARG_BLU );
    
   signal s_idx       : unsigned(7 downto 0);
   signal s_rgb       : std_logic_vector(23 downto 0);
   signal s_write_clk : std_logic;
   signal s_state     : T_STATE;

begin
   cmd_rgblut: process (i_reset_n, i_clk)
   
   begin
      if i_reset_n = '0' then
         s_state          <= CMD;
         s_write_clk      <= '0';
         o_busy           <= '0';
         o_need_more_data <= '0';
            
      elsif rising_edge(i_clk) then
               
         s_write_clk <= '0';                              
         case s_state is
         when CMD =>
            if i_data_rdy = '1' and i_data = X"10" then
               o_busy <= '1';
               o_need_more_data <= '1';
               s_state <= ARG_IDX;
            end if;
         when ARG_IDX =>
            if i_data_rdy = '1' then
               s_idx <= unsigned(i_data);
               s_state <= ARG_RED;
            end if;
         when ARG_RED =>
            if i_data_rdy = '1' then
               s_rgb(7 downto 0) <= i_data;
               s_state <= ARG_GRN;
            end if;
         when ARG_GRN =>
            if i_data_rdy = '1' then
               s_rgb(15 downto 8) <= i_data;
               s_state <= ARG_BLU;
            end if;
         when ARG_BLU =>
            if i_data_rdy = '1' then
               s_rgb(23 downto 16) <= i_data;
               s_state <= CMD;
               o_need_more_data <= '0';
               s_write_clk <= '1';                              
            end if;
         end case;
         
         o_write_clk <= s_write_clk;
         
      end if;      
     
   end process;   
end architecture cmd_fill_arch;
            
            
            
            