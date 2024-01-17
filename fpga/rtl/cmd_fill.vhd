library IEEE;
use IEEE.STD_LOGIC_1164.all;
use IEEE.NUMERIC_STD.all;

entity cmd_fill is
   port (   
      i_reset_n        : in std_logic;
      i_clk            : in std_logic;
      i_data           : in std_logic_vector(7 downto 0);
      i_data_rdy       : in std_logic; -- indication that new data is available
      i_writing        : in std_logic; -- indication that address write is in progress
      --
      o_busy           : out std_logic; -- indication that command is in progress
      o_need_more_data : out std_logic; -- indication that more data is needed
      o_address        : out unsigned(15 downto 0);  -- address to write the RGB values
      o_rgb            : out std_logic_vector(23 downto 0);
      o_write_clk      : out std_logic  -- on rising edge, address and rgb values are ready for writing
   );
end entity cmd_fill;

-- command fills a rectangular area with an RGB color
-- the rectangle is maximum 256x256
-- the generated addess is 16 bits. depending on the used area (e.g. 128x64) the address must be modified by the client.
-- expected byte sequence: 2,<x>,<y>,<dx>,<dy>,<red>,<green>,<blue>
architecture cmd_fill_arch of cmd_fill is   

   type T_STATE is ( CMD, ARG_X, ARG_Y, ARG_DX, ARG_DY, ARG_RED, ARG_GRN, ARG_BLU, WRITE_PIXELS, WAIT_WRITE_START, WAIT_WRITE_DONE );
    
   signal s_x         : unsigned(7 downto 0);
   signal s_px        : unsigned(7 downto 0);
   signal s_py        : unsigned(7 downto 0);
   signal s_dx        : unsigned(7 downto 0);
   signal s_dy        : unsigned(7 downto 0);
   signal s_rgb       : std_logic_vector(23 downto 0);
   signal s_write_clk : std_logic;
   signal s_state     : T_STATE;

begin
   cmd_fill: process (i_reset_n, i_clk)
   
   begin
      if i_reset_n = '0' then
         s_state          <= CMD;
         s_write_clk      <= '0';
         o_busy           <= '0';
         o_need_more_data <= '0';
            
      elsif rising_edge(i_clk) then
               
         case s_state is
         when CMD =>
            if i_data_rdy = '1' and i_data = X"02" then
               o_busy <= '1';
               o_need_more_data <= '1';
               s_state <= ARG_X;
            end if;
         when ARG_X =>
            if i_data_rdy = '1' then
               s_x <= unsigned(i_data);
               s_px <= unsigned(i_data);
               s_state <= ARG_Y;
            end if;
         when ARG_Y =>
            if i_data_rdy = '1' then
               s_py <= unsigned(i_data);
               s_state <= ARG_DX;
            end if;
         when ARG_DX =>
            if i_data_rdy = '1' then
               s_dx <= unsigned(i_data);
               s_state <= ARG_DY;
            end if;
         when ARG_DY =>
            if i_data_rdy = '1' then
               s_dy <= unsigned(i_data);
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
               s_state <= WRITE_PIXELS;
               o_need_more_data <= '0';
               if s_dy = 0 or s_dx = 0 then     -- done when empty rectangle
                  s_state <= CMD;
                  o_busy <= '0';
               end if;
            end if;
         when WRITE_PIXELS =>
            o_address <= s_py & s_px;
            o_rgb <= s_rgb;
            s_write_clk <= '1';                              
            -- prepare next pixel
            s_px <= s_px + 1;
            if s_px = s_x + s_dx - 1 then
               s_px <= s_x;
               s_py <= s_py + 1;
               s_dy <= s_dy - 1;
            end if;   
            s_state <= WAIT_WRITE_START;
         when WAIT_WRITE_START =>
            if i_writing = '1' then
               s_write_clk <= '0';
               s_state <= WAIT_WRITE_DONE;
            end if;
         when WAIT_WRITE_DONE =>
            if i_writing = '0' then
               if s_dy = 0 then     -- check if done with rectangle
                  s_state <= CMD;
                  o_busy <= '0';
               else
                  s_state <= WRITE_PIXELS;
               end if;
            end if;
         end case;
         
         o_write_clk <= s_write_clk;
         
      end if;      
     
   end process;   
end architecture cmd_fill_arch;
            
            
            
            